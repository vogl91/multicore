#define DBG(str) std::cout << str << std::endl;
// #define DBG(str)

#include <mpi.h>

#include <boost/timer/timer.hpp>

#include <cassert>
#include <cmath>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>

#include "matrix.h"
using Matrix_t = Matrix<double>;

constexpr int MASTER_RANK = 0;
constexpr double epsilon = 1e-4;
#define COMPARE_WITH_SEQUENTIAL 0
#define NON_BLOCKING 1

/*========* MATRIX HELPER *========*/

std::ostream& operator<<(std::ostream& os, const Matrix_t& m) {
  const double max = *std::max_element(m.begin(), m.end());
  const size_t max_length = std::max(1.0, log10(max));

  for (auto i = 0u; i < m.height(); ++i) {
    for (auto j = 0u; j < m.width(); ++j) {
      os << std::fixed << std::right << std::setw(max_length + 4)
         << std::setprecision(2);
      os << m(i, j) << " ";
    }
    os << std::endl;
  }
  return os;
}

void fill_random(Matrix_t& m) {
  std::random_device real_random;
  std::mt19937 random{real_random()};
  std::uniform_real_distribution<Matrix_t::value_t> distribution{0, 10};
  auto next_val = std::bind(distribution, random);
  std::generate(m.begin(), m.end(), next_val);
}

using namespace std;

/*========* HELPER *========*/

static Matrix_t::value_t four_cell_neighbour_sum(const Matrix_t& m, size_t i,
                                                 size_t j) {
  assert(i < m.height() - 1);
  assert(j < m.width() - 1);
  return m(i + 1, j) + m(i - 1, j) + m(i, j + 1) + m(i, j - 1);
}

/*========* SEQUENTIAL VERSION *========*/

static double deviation(const Matrix_t& u_new, const Matrix_t& u_old) {
  assert(u_old.height() == u_old.height());
  assert(u_old.width() == u_new.width());

  double diff = 0;
  for (auto i = 1u; i < u_old.height() - 1; ++i) {
    for (auto j = 1u; j < u_old.width() - 1; ++j) {
      diff += (u_new(i, j) - u_old(i, j)) * (u_new(i, j) - u_old(i, j));
    }
  }
  return sqrt(diff);
}

Matrix_t jacobi(Matrix_t u_old, double epsilon) {
  assert(u_old.width() == u_old.height());
  Matrix_t u_new = u_old;

  const auto N = u_old.height() - 1;
  size_t loop_count = 0;
  while (true) {
    // for (int i = 0; i < 1000; ++i) {
    ++loop_count;
    for (auto i = 1u; i < N; ++i) {
      for (auto j = 1u; j < N; ++j) {
        u_new(i, j) = four_cell_neighbour_sum(u_old, i, j) / 4;
      }
    }
    if (deviation(u_new, u_old) < epsilon) break;
    Matrix_t& tmp = u_old;
    u_old = u_new;
    u_new = tmp;
  }
  DBG("iterations:" << loop_count << endl);
  return u_new;
}

/*========* PARALLEL VERSION *========*/

class Parallel_jacobi {
 public:
  Parallel_jacobi(int rank, int numtasks, Matrix_t& m)
      : rank{rank}, numtasks{numtasks}, u_old{m}, u_new{m} {
    size_t loop_count = 0;
    while (true) {
      ++loop_count;
      // for (int i = 0; i < 1000; ++i) {
      exchange_borders();
      for (auto i = 1u; i < u_new.height() - 1; ++i) {
        for (auto j = 1u; j < u_new.width() - 1; ++j) {
          u_new(i, j) = four_cell_neighbour_sum(u_old, i, j) / 4;
        }
      }
      swap(u_old, u_new);
      if (is_finish()) break;
    }
    DBG("iterations:" << loop_count << endl);
  }

 private:
  double local_squared_diffsum() const {
    double diff = 0;
    for (auto i = 1u; i < u_old.height() - 1; ++i) {
      for (auto j = 1u; j < u_old.width() - 1; ++j) {
        diff += (u_new(i, j) - u_old(i, j)) * (u_new(i, j) - u_old(i, j));
      }
    }
    return diff;
  }

  bool is_finish() {
    const auto diff = local_squared_diffsum();
    double deviation;
    MPI::COMM_WORLD.Reduce(&diff, &deviation, 1, MPI::DOUBLE, MPI::SUM,
                           MASTER_RANK);
    bool finish;
    if (rank == MASTER_RANK) {
      deviation = sqrt(deviation);
      finish = deviation < epsilon;
    }
    MPI::COMM_WORLD.Bcast(&finish, 1, MPI::BOOL, MASTER_RANK);
    return finish;
  }
  void exchange_upper() {
    if (rank > 0) {
      MPI::COMM_WORLD.Sendrecv(
          u_old.begin_row(1), u_old.width(), MPI_DOUBLE, rank - 1, 0,  //
          u_old.begin_row(0), u_old.width(), MPI_DOUBLE, rank - 1, 0);
    }
  }

  void exchange_lower() {
    if (rank != numtasks - 1) {
      MPI::COMM_WORLD.Sendrecv(u_old.begin_row(u_old.height() - 2),
                               u_old.width(), MPI_DOUBLE, rank + 1, 0,  //
                               u_old.begin_row(u_old.height() - 1),
                               u_old.width(), MPI_DOUBLE, rank + 1, 0);
    }
  }

  void exchange_borders() {
#if NON_BLOCKING
    MPI::Request requests[4] = {MPI::REQUEST_NULL, MPI::REQUEST_NULL,
                                MPI::REQUEST_NULL, MPI::REQUEST_NULL};
    if (rank > 0) {
      requests[0] = MPI::COMM_WORLD.Irecv(u_old.begin_row(0), u_old.width(),
                                          MPI::DOUBLE, rank - 1, 0);
    }
    if (rank != numtasks - 1) {
      requests[1] =
          MPI::COMM_WORLD.Irecv(u_old.begin_row(u_old.height() - 1),
                                u_old.width(), MPI_DOUBLE, rank + 1, 0);
    }
    if (rank > 0) {
      requests[2] = MPI::COMM_WORLD.Isend(u_old.begin_row(1), u_old.width(),
                                          MPI_DOUBLE, rank - 1, 0);
    }
    if (rank != numtasks - 1) {
      requests[3] =
          MPI::COMM_WORLD.Isend(u_old.begin_row(u_old.height() - 2),
                                u_old.width(), MPI_DOUBLE, rank + 1, 0);
    }
    MPI::Request::Waitall(4, requests);

#else
    if (rank % 2 == 0) {
      exchange_upper();
      exchange_lower();
    } else {
      exchange_lower();
      exchange_upper();
    }
#endif
  }

 private:
  const int rank;
  const int numtasks;
  Matrix_t& u_old;
  Matrix_t u_new;
};

void main_master() {
  DBG("starting master...");

  // const size_t N = 11;
  const size_t N = 100;
  Matrix_t m{N, N};
  fill_random(m);

#if COMPARE_WITH_SEQUENTIAL
  Matrix_t mcopy = m;
// cout << m << endl;
#endif

  const auto numtasks = MPI::COMM_WORLD.Get_size();
  const auto rows_per_task = N / (numtasks - 1);
  const auto chunk_size = rows_per_task * N;
  const auto master_rows = N - (numtasks - 1) * rows_per_task;
  const auto master_chunk_size = master_rows * N;

  {  // send chunk sizes
    int buffer[2] = {N, static_cast<int>(rows_per_task)};
    MPI::COMM_WORLD.Bcast(&buffer, 2, MPI_INT, MASTER_RANK);
  }
  // send matrix data
  for (auto i = 1; i < numtasks; ++i) {
    auto data = m.begin_row(master_rows + (i - 1) * rows_per_task);
    MPI::COMM_WORLD.Send(data, chunk_size, MPI_DOUBLE, i, 0);
  }

  {  // calculate
    Matrix_t m2{master_rows + 1, N};
    std::copy(m.begin_row(0), m.begin_row(0) + master_chunk_size, m2.begin());
    Parallel_jacobi{MASTER_RANK, numtasks, m2};
    std::copy(m2.begin(), m2.end(), m.begin());
  }

  // receive result data
  for (auto i = 1; i < numtasks; ++i) {
    auto data = m.begin_row(master_rows + (i - 1) * rows_per_task);
    MPI::COMM_WORLD.Recv(data, chunk_size, MPI_DOUBLE, i, 0);
  }

// cout << m << endl;

#if COMPARE_WITH_SEQUENTIAL
  mcopy = jacobi(mcopy, epsilon);
  cout << "squential == parallel: " << (m == mcopy ? "true" : "false") << endl;
// cout << endl << mcopy << endl;
#endif
  DBG("shutting down master...");
}

void main_slave(int rank) {
  DBG("starting slave " << rank << "...");

  const auto numtasks = MPI::COMM_WORLD.Get_size();
  size_t N, rows_per_task;
  {
    int buffer[2];
    MPI::COMM_WORLD.Bcast(&buffer, 2, MPI_INT, MASTER_RANK);
    N = buffer[0];
    rows_per_task = buffer[1];
  }
  const auto is_last = rank == numtasks - 1;
  const auto chunk_size = rows_per_task * N;
  Matrix_t m{rows_per_task + (is_last ? 1 : 2), N};
  MPI::COMM_WORLD.Recv(m.begin_row(1), chunk_size, MPI_DOUBLE, MASTER_RANK, 0);

  Parallel_jacobi{rank, numtasks, m};

  MPI::COMM_WORLD.Send(m.begin_row(1), chunk_size, MPI_DOUBLE, MASTER_RANK, 0);
  DBG("shutting down slave " << rank << "...");
}
int main(int argc, char* argv[]) {
  boost::timer::auto_cpu_timer t;
  MPI::Init(argc, argv);

  const int rank = MPI::COMM_WORLD.Get_rank();

  if (rank == MASTER_RANK) {
    main_master();
  } else {
    main_slave(rank);
  }

  MPI::Finalize();
  return 0;
}
