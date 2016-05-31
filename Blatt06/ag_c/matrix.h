#ifndef MATRIX_H
#define MATRIX_H

#include <cassert>
#include <cstddef>

#include <memory>

template <typename T>
class Matrix;

template <typename T>
void swap(Matrix<T>& m, Matrix<T>& n);

template <typename T>
class Matrix {
 public:
  using value_t = T;

 public:
  Matrix() : height_{0}, width_{0}, data_{nullptr} {}
  Matrix(size_t height, size_t width)
      : height_(height), width_(width), data_{new value_t[height * width]} {}
  Matrix(const Matrix& that) : Matrix{that.height(), that.width()} {
    copy_data(that.data());
  }
  Matrix(Matrix&&) = default;
  Matrix(size_t height, size_t width, const T* data) : Matrix{height, width} {
    copy_data(data);
  }

  Matrix& operator=(const Matrix& that) {
    if (height() != that.height() || width() != that.width()) {
      height_ = that.height();
      width_ = that.width();
      data_ =
          std::unique_ptr<value_t[]>{new value_t[that.height() * that.width()]};
    }
    copy_data(that.data());
    return *this;
  }
  Matrix& operator=(Matrix&&) = default;

 public:
  size_t height() const { return height_; }
  size_t width() const { return width_; }
  value_t* data() { return data_.get(); }
  const value_t* data() const { return data_.get(); }
  value_t& get(size_t i, size_t j) {
    assert(i < height());
    assert(j < width());
    return data_[index(i, j)];
  }
  const value_t& get(size_t i, size_t j) const {
    assert(i < height());
    assert(j < width());
    return data_[index(i, j)];
  }
  value_t& operator()(size_t i, size_t j) { return get(i, j); }
  const value_t& operator()(size_t i, size_t j) const { return get(i, j); }
  value_t* begin() { return data(); }
  const value_t* begin() const { return data(); }
  value_t* end() { return data() + height() * width(); }
  const value_t* end() const { return data() + height() * width(); }

  value_t* begin_row(size_t i) {
    assert(i < height());
    return data() + i * width();
  }
  const value_t* begin_row(size_t i) const {
    assert(i < height());
    return data() + i * width();
  }
  value_t* end_row(size_t i) {
    assert(i >= 0);
    assert(i < height());
    return data() + (i + 1) * width() - 1;
  }
  const value_t* end_row(size_t i) const {
    assert(i >= 0);
    assert(i < height());
    return data() + (i + 1) * width() - 1;
  }

 public:
  friend void swap<T>(Matrix& m, Matrix& n);

 private:
  size_t height_;
  size_t width_;
  std::unique_ptr<value_t[]> data_;

 private:
  size_t index(size_t i, size_t j) const { return i * width_ + j; }
  void copy_data(const T* data) {
    std::copy(data, data + height() * width(), this->data());
  }
};

template <typename T>
void swap(Matrix<T>& m, Matrix<T>& n) {
  std::swap(m.height_, n.height_);
  std::swap(m.width_, n.width_);
  std::swap(m.data_, n.data_);
}

template <typename T>
bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs) {
  if (lhs.height() != rhs.height()) return false;
  if (lhs.width() != rhs.width())
    return false;
  else
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs) {
  return !(lhs == rhs);
}

#endif /*MATRIX_H*/