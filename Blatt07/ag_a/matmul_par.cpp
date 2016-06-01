#include <cassert>

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

#include "matrix.h"

using namespace std;

void join_all(vector<thread>& threads) {
	for (auto& t : threads) {
		if (t.joinable())
			t.join();
	}
}

void mult(MatrixBlock& c, const MatrixBlock& a, const MatrixBlock& b) {
	assert(a.width() == b.height());
	assert(c.height() == a.height());
	assert(c.width() == b.width());

	const auto N = a.height();
	const auto M = a.width();
	const auto P = b.width();

	for (auto i = 0u; i < N; ++i) {
		for (auto j = 0u; j < P; ++j) {
			float x = 0;
			for (auto k = 0u; k < M; ++k) {
				x += a(i, k) * b(k, j);
			}
			c(i, j) = x;
		}
	}
}


void foo() {}

Matrix mult(Matrix& a, Matrix& b) {
	assert(a.size() == b.size());
	Matrix c { a.size() };

	vector<thread> threads;
	for (auto i = 0u; i < 4; ++i) {
		for (auto j = 0u; j < 4; ++j) {
			threads.push_back(thread { [&](size_t i,size_t j) {
				auto starti = i*2;
				auto startj = j*2;
				MatrixBlock ba {a, starti, startj, 2, 2};
				MatrixBlock bb {b, starti, startj, 2, 2};
				MatrixBlock bc {c, starti, startj, 2, 2};
				mult(bc,ba,bb);
			}, i, j });
		}
	}

	join_all(threads);
	return c;
}

int main(int argc, char *argv[]) {
	const auto N = 8;
	Matrix a { N };
	Matrix b { N };
	fill_random(a, 0, 1);
	fill_random(b, 0, 1);

	Matrix c = mult(a, b);

	cout << a << endl << b << endl;
	cout << c << endl;
	return 0;
}
