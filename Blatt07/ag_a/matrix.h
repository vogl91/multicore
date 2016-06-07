#ifndef MATRIX_H
#define MATRIX_H

#include <cassert>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <istream>
#include <ostream>
#include <random>

/*========* MATRIX CLASS *========*/

template <typename T>
class Matrix {
 public:
  Matrix(size_t height, size_t width)
      : height_{height}, width_{width}, data_{new float[height * width]} {}
  Matrix(size_t height, size_t width, T init) : Matrix{height, width} {
    std::fill(begin(), end(), init);
  }
  ~Matrix() { delete[] data_; }
  size_t height() const { return height_; }
  size_t width() const { return width_; }
  T* data() { return data_; }
  const T* data() const { return data_; }
  T* begin() { return data_; }
  T* end() { return data_ + height_ * width_; }
  const T* begin() const { return data_; }
  const T* end() const { return data_ + height_ * width_; }
  T& get(size_t i, size_t j) { return data_[index(i, j)]; }
  const T& get(size_t i, size_t j) const { return data_[index(i, j)]; }
  T& operator()(size_t i, size_t j) { return get(i, j); }
  const T& operator()(size_t i, size_t j) const { return get(i, j); }

 private:
  size_t index(size_t i, size_t j) const { return i * width_ + j; }

 private:
  size_t height_;
  size_t width_;
  float* data_;
};

template <typename T>
bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename T>
bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs) {
  return !(*lhs == rhs);
}

/*========* MATRIX HELPER *========*/

std::ostream& operator<<(std::ostream& os, const Matrix<float>& m) {
  const double max = *std::max_element(m.begin(), m.end());
  const size_t max_length = std::max(1.0, log10(max));

  const auto N = m.height();
  const auto M = m.width();
  for (auto i = 0u; i < N; ++i) {
    for (auto j = 0u; j < M; ++j) {
      os << std::fixed << std::right << std::setw(max_length + 4)
         << std::setprecision(2);
      os << m(i, j) << " ";
    }
    os << std::endl;
  }
  return os;
}

void fill_random(Matrix<float>& m, float min, float max) {
  std::random_device real_random;
  std::mt19937 random{real_random()};
  std::uniform_real_distribution<float> distribution{min, max};
  auto next_val = std::bind(distribution, random);
  generate(m.begin(), m.end(), next_val);
}

void fill_random(Matrix<float>& m, int min, int max) {
  std::random_device real_random;
  std::mt19937 random{real_random()};
  std::uniform_int_distribution<int> distribution{min, max};
  auto next_val = std::bind(distribution, random);
  generate(m.begin(), m.end(), next_val);
}

#endif /* MATRIX_H */
