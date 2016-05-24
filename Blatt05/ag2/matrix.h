#ifndef MATRIX_H
#define MATRIX_H

#include <cassert>

#include <algorithm>
#include <initializer_list>
#include <memory>

template <typename T>
struct Matrix {
  using value_t = T;
public:
  Matrix(size_t height, size_t width) : width_(width), height_(height) {
    data_ = std::unique_ptr<T[]>{new T[width * height]};
  }
  Matrix(size_t height, size_t width, const void* values)
      : Matrix{height, width} {
    memcpy(data(), values, width * height * sizeof(T));
  }
  Matrix(size_t height, size_t width,
         std::initializer_list<std::initializer_list<T>> list)
      : Matrix{height, width} {
    assert(height == list.size());
    auto it = data();
    for (auto& inner_list : list) {
      assert(width == inner_list.size());
      it = std::copy(inner_list.begin(), inner_list.end(), it);
    }
  }
  Matrix(const Matrix& that)
      : Matrix{that.height(), that.width(), that.data()} {}
  Matrix(Matrix&& that) = default;

  inline size_t width() const { return width_; }
  inline size_t height() const { return height_; }
  inline T* data() { return data_.get(); }
  inline const T* data() const { return data_.get(); }

  T& operator()(size_t i, size_t j) { return get(i, j); }
  const T& operator()(size_t i, size_t j) const { return get(i, j); }
  T* begin() { return data(); }
  const T* begin() const { return data(); }
  T* end() { return data() + width_ * height_; }
  const T* end() const { return data() + width_ * height_; }

  Matrix transposed() const {
    Matrix result{width(), height()};
    for (auto i = 0u; i < height(); ++i) {
      for (auto j = 0u; j < width(); ++j) {
        result(j, i) = get(i, j);
      }
    }
    return result;
  }

  template <typename U>
  friend void partial_mult(Matrix<U>& C, const Matrix<U>& A,
                           const Matrix<U>& BTransposed, size_t start_index,
                           size_t end_index);

 private:
  size_t width_, height_;
  std::unique_ptr<T[]> data_;

 private:
  inline size_t index(size_t i, size_t j) const { return i * width_ + j; }
  inline T& get(size_t i, size_t j) { return data_[index(i, j)]; }
  inline const T& get(size_t i, size_t j) const { return data_[index(i, j)]; }
};

#include <iostream>
#include <iomanip>
#include <string>
template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& m) {
  const auto max_val = *std::max_element(std::begin(m), std::end(m));
  const auto width = std::to_string(max_val).length();
  for (auto i = 0u; i < m.height(); ++i) {
    for (auto j = 0u; j < m.width(); ++j) {
      os << std::setw(width) << m(i, j) << " ";
    }
    os << std::endl;
  }
  return os;
}

template <typename T>
void mult(Matrix<T>& C, const Matrix<T>& A, const Matrix<T>& B) {
  assert(A.width() == B.height());
  assert(A.height() == C.height());
  assert(B.width() == C.width());
  const auto N = A.height();
  const auto M = A.width();
  const auto P = B.width();

  for (auto i = 0u; i < N; ++i) {
    for (auto j = 0u; j < P; ++j) {
      auto c_i_j = 0;
      for (auto k = 0u; k < M; ++k) {
        c_i_j += A(i, k) * B(k, j);
      }
      C(i, j) = c_i_j;
    }
  }
}

template <typename T, typename Iter>
T zip_acc(Iter first1, Iter first2, Iter last1, Iter last2) {
  T result = 0;
  while (first1 != last1 && first2 != last2) {
    result += *first1 * *first2;
    ++first1, ++first2;
  }
  return result;
}

#include <iostream>
using namespace std;

template <typename Iter>
void print(Iter first, Iter last) {
  while (first != last) {
    cout << *first << " ";
    ++first;
  }
  cout << endl;
}

template <typename T>
void partial_mult(Matrix<T>& C, const Matrix<T>& A,
                  const Matrix<T>& BTransposed, size_t start_index,
                  size_t end_index) {
  assert(start_index <= end_index);
  const auto chuck_size = A.width();
  size_t w = start_index % C.width(), h = start_index / A.width();
  for (auto i = start_index; i < end_index; ++i) {
    auto firstA = A.data() + A.index(h, 0);
    auto lastA = firstA + chuck_size;
    auto firstB = BTransposed.data() + BTransposed.index(w, 0);
    auto lastB = firstB + chuck_size;
    // print(firstA, lastA);
    // print(firstB, lastB);
    // cout << endl;
    C.data_[i] = zip_acc<T>(firstA, firstB, lastA, lastB);
    ++w;
    if (w == C.width()) {
      w = 0;
      ++h;
    }
  }
}

#endif /*MATRIX_H*/