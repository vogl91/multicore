#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>

#include "matrix.h"

std::ostream& operator<<(std::ostream& os, const Matrix<double>& m) {
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

std::istream& operator>>(std::istream& is, Matrix<double>& m) {
  for (auto i = 0u; i < m.height(); ++i) {
    for (auto j = 0u; j < m.width(); ++j) {
      is >> m(i, j);
    }
  }
  return is;
}

void fill_random(Matrix<double>& m) {
  std::random_device real_random;
  std::mt19937 random{real_random()};
  std::uniform_real_distribution<Matrix<double>::value_t> distribution{0, 10};
  auto next_val = std::bind(distribution, random);
  std::generate(m.begin(), m.end(), next_val);
}

using namespace std;

int main(int argc, char const* argv[]) {
  if (argc != 2) {
    cerr << "wrong number of params" << endl;
    return 1;
  }
  const size_t N = atoi(argv[1]);
  Matrix<double> m{N, N};
  fill_random(m);
  cout << m << endl;
  return 0;
}