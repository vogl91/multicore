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

#define DBG(str) std::cout << (str) << std::endl;

constexpr int MASTER_RANK = 0;

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

static Matrix_t::value_t four_cell_neighbour_sum(const Matrix_t& m, size_t i,
                                                 size_t j) {
  assert(i < m.height() - 1);
  assert(j < m.width() - 1);
  return m(i + 1, j) + m(i - 1, j) + m(i, j + 1) + m(i, j - 1);
}

Matrix_t jacobi(Matrix_t u_old, double epsilon) {
  assert(u_old.width() == u_old.height());
  Matrix_t u_new = u_old;

  const auto N = u_old.height() - 1;
  size_t loop_count = 0;
  // while (true) {
  for(int i=0;i<1000;++i) {
    ++loop_count;
    for (auto i = 1u; i < N; ++i) {
      for (auto j = 1u; j < N; ++j) {
        u_new(i, j) = four_cell_neighbour_sum(u_old, i, j) / 4;
      }
    }
    // if (deviation(u_new, u_old) < epsilon) break;
    Matrix_t& tmp = u_old;
    u_old = u_new;
    u_new = tmp;
  }
  cout << "iterations:" << loop_count << endl << endl;
  return u_new;
}

void exchange_upper(Matrix_t& m, int rank) {
  if (rank > 0) {
    MPI::COMM_WORLD.Sendrecv(
        m.begin_row(1), m.width(), MPI_DOUBLE, rank - 1, 0,  //
        m.begin_row(0), m.width(), MPI_DOUBLE, rank - 1, 0);
  }
}

void exchange_lower(Matrix_t& m, int rank, int numtasks) {
  if (rank != numtasks - 1) {
    MPI::COMM_WORLD.Sendrecv(
        m.begin_row(m.height() - 2), m.width(), MPI_DOUBLE, rank + 1, 0,  //
        m.begin_row(m.height() - 1), m.width(), MPI_DOUBLE, rank + 1, 0);
  }
}

void exchange_borders(Matrix_t& m, int rank, int numtasks) {
  if (rank % 2 == 0) {
    exchange_upper(m, rank);
    exchange_lower(m, rank, numtasks);
  } else {
    exchange_lower(m, rank, numtasks);
    exchange_upper(m, rank);
  }
}

constexpr double epsilon = 0.0001;
void do_jacobi(Matrix_t& u_old, int rank) {
  const auto numtasks = MPI::COMM_WORLD.Get_size();

  Matrix_t u_new = u_old;

  // while (true) {
  for(int i=0;i<1000;++i) {
    exchange_borders(u_old, rank, numtasks);
    for (auto i = 1u; i < u_new.height() - 1; ++i) {
      for (auto j = 1u; j < u_new.width() - 1; ++j) {
        u_new(i, j) = four_cell_neighbour_sum(u_old, i, j) / 4;
      }
    }
    auto delta = deviation(u_new, u_old);
    swap(u_old, u_new);
    // if (delta < epsilon) break;
  }
}

void main_master() {
  cout << "starting master..." << endl;

  const size_t N = 11;
  Matrix_t m{N, N};
  fill_random(m);

  Matrix_t mcopy = m;
  cout << m << endl;

  const auto numtasks = MPI::COMM_WORLD.Get_size();
  const auto rows_per_task = N / numtasks + (N % numtasks != 0);
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
    do_jacobi(m2, MASTER_RANK);
    std::copy(m2.begin(), m2.end(), m.begin());
  }

  // receive result data
  for (auto i = 1; i < numtasks; ++i) {
    auto data = m.begin_row(master_rows + (i - 1) * rows_per_task);
    MPI::COMM_WORLD.Recv(data, chunk_size, MPI_DOUBLE, i, 0);
  }

  cout << m << endl;

  mcopy = jacobi(mcopy,0.0001);
  cout << (m == mcopy) << endl;
}

void main_slave(int rank) {
  cout << "starting slave " << rank << "..." << endl;

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

  do_jacobi(m, rank);

  MPI::COMM_WORLD.Send(m.begin_row(1), chunk_size, MPI_DOUBLE, MASTER_RANK, 0);
  cout << "shutting down slave " << rank << "..." << endl;
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
