#include <mpi.h>

#include <boost/timer/timer.hpp>

#include <cassert>
#include <cmath>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>

/*========* Matrix *========*/

class Matrix {
 public:
  using value_t = double;

 public:
  Matrix(size_t height, size_t width)
      : height_(height), width_(width), data_{new value_t[height * width]} {}
  Matrix(const Matrix& that) : Matrix{that.height(), that.width()} {
    copy_data(that);
  }
  Matrix(Matrix&&) = default;
  Matrix& operator=(const Matrix& that) {
    copy_data(that);
    return *this;
  }
  Matrix& operator=(Matrix&&) = default;

 public:
  size_t height() const { return height_; }
  size_t width() const { return width_; }
  value_t* data() { return data_.get(); }
  const value_t* data() const { return data_.get(); }
  value_t& get(size_t i, size_t j) { return data_[index(i, j)]; }
  const value_t& get(size_t i, size_t j) const { return data_[index(i, j)]; }
  value_t& operator()(size_t i, size_t j) { return get(i, j); }
  const value_t& operator()(size_t i, size_t j) const { return get(i, j); }
  value_t* begin() { return data(); }
  const value_t* begin() const { return data(); }
  value_t* end() { return data() + height() * width(); }
  const value_t* end() const { return data() + height() * width(); }

 public:
  friend void swap(Matrix& m, Matrix& n);

 private:
  size_t height_;
  size_t width_;
  std::unique_ptr<value_t[]> data_;

 private:
  size_t index(size_t i, size_t j) const { return i * width_ + j; }
  void copy_data(const Matrix& that) {
    std::copy(that.data(), that.data() + height() * width(), data());
  }
};

void swap(Matrix& m, Matrix& n) {
  const auto tmp_height = m.height_;
  const auto tmp_width = m.width_;
  m.height_ = n.height_;
  m.width_ = n.width_;
  n.height_ = tmp_height;
  n.width_ = tmp_width;
  std::swap(m.data_, n.data_);
}

// void Func(const Matrix& m, size_t i, size_t j)
template <typename Func>
void for_each_cell_index(const Matrix& m, Func f) {
  for (auto i = 0u; i < m.height(); ++i) {
    for (auto j = 0u; j < m.height(); ++j) {
      f(m, i, j);
    }
  }
}

// void Func(Matrix& m, size_t i, size_t j)
template <typename Func>
void for_each_cell_index(Matrix& m, Func f) {
  for (auto i = 0u; i < m.height(); ++i) {
    for (auto j = 0u; j < m.height(); ++j) {
      f(m, i, j);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
  const double max = *std::max_element(m.begin(), m.end());
  const size_t max_length = std::max(1.0, log10(max));

  for_each_cell_index(m,
                      [&os, max_length](const Matrix& m, size_t i, size_t j) {
                        os << std::fixed << std::right
                           << std::setw(max_length + 4) << std::setprecision(2);
                        os << m(i, j) << " ";
                        if (j == m.width() - 1) os << std::endl;
                      });
  return os;
}

void fill_random(Matrix& m) {
  std::random_device real_random;
  std::mt19937 random{real_random()};
  std::uniform_real_distribution<Matrix::value_t> distribution{0, 10};
  auto next_val = std::bind(distribution, random);
  for_each_cell_index(
      m, [&next_val](Matrix& m, size_t i, size_t j) { m(i, j) = next_val(); });
}

/*========* Main *========*/

using namespace std;

static double deviation(const Matrix& u_new, const Matrix& u_old) {
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

static Matrix::value_t four_cell_neighbour_sum(const Matrix& m, size_t i,
                                               size_t j) {
  assert(i < m.height() - 1);
  assert(j < m.width() - 1);
  return m(i + 1, j) + m(i - 1, j) + m(i, j + 1) + m(i, j - 1);
}

Matrix jacobi(Matrix u_old, double epsilon) {
  assert(u_old.width() == u_old.height());
  Matrix u_new = u_old;

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
    Matrix& tmp = u_old;
    u_old = u_new;
    u_new = tmp;
  }
  cout << "iterations:" << loop_count << endl << endl;
  return u_new;
}

void do_jacobi() {
  boost::timer::auto_cpu_timer t;
  const size_t N = 6;
  Matrix m{N, N};
  fill_random(m);

  cout << m << endl;
  auto result = jacobi(m, 0.0000000001);
  cout << result << endl;
  
}

#include <vector>

int main(int argc, char* argv[]) {
  do_jacobi();
  return 0;
}
