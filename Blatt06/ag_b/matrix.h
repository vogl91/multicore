#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>

#include <memory>

template<typename T>
class Matrix;

template <typename T>
void swap(Matrix<T>& m, Matrix<T>& n);

template <typename T>
class Matrix {
 public:
  using value_t = T;

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
  friend void swap<T>(Matrix& m, Matrix& n);

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

template <typename T>
void swap(Matrix<T>& m, Matrix<T>& n) {
  std::swap(m.height_,n.height_);
  std::swap(m.width_,n.width_);
  std::swap(m.data_, n.data_);
}

#endif /*MATRIX_H*/