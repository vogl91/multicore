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
  assert(u_old.width() == u_old.height());
  assert(u_new.width() == u_new.height());
  assert(u_old.width() == u_new.width());

  const auto N = u_old.height() - 1;
  double diff = 0;
  for (auto i = 1u; i < N; ++i) {
    for (auto j = 1u; j < N; ++j) {
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
  while (true) {
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
  cout << "iterations:" << loop_count << endl << endl;
  return u_new;
}

void do_jacobi() {
  boost::timer::auto_cpu_timer t;
  const size_t N = 6;
  Matrix_t m{N, N};
  fill_random(m);

  cout << m << endl;
  auto result = jacobi(m, 0.0000000001);
  cout << result << endl;
}

void test_swap() {
  Matrix_t m{2, 3};
  Matrix_t n{3, 2};
  fill_random(m);
  fill_random(n);
  cout << m << endl;
  cout << n << endl;
  swap(m, n);
  cout << m << endl;
  cout << n << endl;
}

int main(int argc, char* argv[]) {
  do_jacobi();

  return 0;
}
