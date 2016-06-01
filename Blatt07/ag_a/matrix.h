/*
 * matrix.h
 *
 *  Created on: 01.06.2016
 *      Author: vogelgal
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <algorithm>
#include <iomanip>
#include <functional>
#include <ostream>
#include <istream>
#include <random>

/*========* MATRIX CLASS *========*/

class Matrix {
public:
	Matrix(size_t size) :
			size_ { size }, data_ { new float[size * size] } {
	}
	~Matrix() {
		delete[] data_;
	}
	size_t size() const {
		return size_;
	}
	float* data() {
		return data_;
	}
	const float* data() const {
		return data_;
	}
	float* begin() {
		return data_;
	}
	float* end() {
		return data_ + size() * size();
	}
	const float* begin() const {
		return data_;
	}
	const float* end() const {
		return data_ + size() * size();
	}
	float& get(size_t i, size_t j) {
		return data_[index(i, j)];
	}
	const float& get(size_t i, size_t j) const {
		return data_[index(i, j)];
	}
	float& operator()(size_t i, size_t j) {
		return get(i, j);
	}
	const float& operator()(size_t i, size_t j) const {
		return get(i, j);
	}
private:
	size_t index(size_t i, size_t j) const {
		return i * size_ + j;
	}
private:
	size_t size_;
	float* data_;
};

/*========* BLOCK MATRIX *========*/

class MatrixBlock {
public:
	struct ConstIterator {
		const MatrixBlock* m;
		size_t i, j;

		ConstIterator& operator++() {
			++j;
			if (j >= m->width()) {
				j = 0;
				++i;
			}
			return *this;
		}
		const float& operator*() {
			return m->get(i, j);
		}
		bool operator==(const ConstIterator& that) const {
			return m == that.m && i == that.i && j == that.j;
		}
		bool operator!=(const ConstIterator& that) const {
			return !(*this == that);
		}
	};
public:
	MatrixBlock(Matrix& m, size_t starti, size_t startj, size_t height,
			size_t width) :
			m { m }, starti_ { starti }, startj_ { startj }, height_ { height }, width_ {
					width } {
		assert(starti + height <= m.size());
		assert(startj + width <= m.size());
	}
	size_t starti() const {
		return starti_;
	}
	size_t startj() const {
		return startj_;
	}
	size_t height() const {
		return height_;
	}
	size_t width() const {
		return width_;
	}
	float& get(size_t i, size_t j) {
		return m(mapi(i), mapj(j));
	}
	const float& get(size_t i, size_t j) const {
		return m(mapi(i), mapj(j));
	}
	float& operator()(size_t i, size_t j) {
		return get(i, j);
	}
	const float& operator()(size_t i, size_t j) const {
		return get(i, j);
	}
	ConstIterator begin() const {
		return ConstIterator { this, 0, 0 };
	}
	ConstIterator end() const {
		return ConstIterator { this, height_, 0 };
	}
private:
	size_t mapi(size_t i) const {
		return i + starti_;
	}
	size_t mapj(size_t j) const {
		return j + startj_;
	}
private:
	Matrix& m;
	size_t starti_, startj_, width_, height_;
};

/*========* MATRIX HELPER *========*/

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
	const double max = *std::max_element(m.begin(), m.end());
	const size_t max_length = std::max(1.0, log10(max));

	const auto N = m.size();
	for (auto i = 0u; i < N; ++i) {
		for (auto j = 0u; j < N; ++j) {
			os << std::fixed << std::right << std::setw(max_length + 4)
					<< std::setprecision(2);
			os << m(i, j) << " ";
		}
		os << std::endl;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const MatrixBlock& b) {
	const double max = *std::max_element(b.begin(), b.end());
	const size_t max_length = std::max(1.0, log10(max));

	for (auto i = 0u; i < b.height(); ++i) {
		for (auto j = 0u; j < b.width(); ++j) {
			os << std::fixed << std::right << std::setw(max_length + 4)
					<< std::setprecision(2);
			os << b(i, j) << " ";
		}
		os << std::endl;
	}
	return os;
}

void fill_random(Matrix& m, float min, float max) {
	std::random_device real_random;
	std::mt19937 random { real_random() };
	std::uniform_real_distribution<float> distribution { min, max };
	auto next_val = std::bind(distribution, random);
	generate(m.begin(), m.end(), next_val);
}

#endif /* MATRIX_H_ */
