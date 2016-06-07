#include <cassert>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "matrix.h"

using FMatrix = Matrix<float>;

using namespace std;

void join_all(vector<thread>& threads) {
  for (auto& t : threads) {
    if (t.joinable()) t.join();
  }
}

struct single_threaded_tag {};
struct multi_threaded_tag {};

static void mult(FMatrix& c, const FMatrix& a, const FMatrix& b,
                 single_threaded_tag) {
  const auto N = c.height();
  const auto M = a.width();
  const auto P = c.width();

  for (auto i = 0u; i < N; ++i) {
    for (auto j = 0u; j < M; ++j) {
      auto sum = 0.0f;
      for (auto k = 0u; k < P; ++k) {
        sum += a(i, k) * b(k, j);
      }
      c(i, j) = sum;
    }
  }
}

static void tile_mult(FMatrix& c, const FMatrix& a, const FMatrix& b,     //
                      const size_t ii, const size_t jj, const size_t kk,  //
                      const size_t N, const size_t P, const size_t M) {
  for (auto i = ii; i < N; ++i) {
    for (auto j = jj; j < P; ++j) {
      float sum = 0.0f;
      for (auto k = kk; k < M; ++k) {
        sum += a(i, k) * b(k, j);
      }
      c(i, j) += sum;
    }
  }
}

static void mult(FMatrix& c, const FMatrix& a, const FMatrix& b,
                 multi_threaded_tag) {
  constexpr auto numthreads = 4;

  const auto N = c.height();
  const auto M = a.width();
  const auto P = c.width();
  assert(("Matrices must be squre", N == M && M == P));
  const auto tile_size = N / numthreads;
  assert(("Matrix size must be divisible by numthreads",
          tile_size * numthreads == N));
  vector<thread> threads;
  for (auto x = 0u; x < numthreads; ++x) {
    threads.push_back(thread{[x, &c, &a, &b, &tile_size, &M, &P]() {
      const auto start_i = x * tile_size;
      const auto end_i = start_i + tile_size;
      for (auto i = start_i; i < end_i; ++i) {
        for (auto j = 0u; j < M; ++j) {
          // calculate c(i,j)
          auto sum = 0.0f;
          for (auto k = 0u; k < P; ++k) {
            sum += a(i, k) * b(k, j);
          }
          c(i, j) = sum;
        }
      }
    }});
  }
  join_all(threads);
}

template <typename Tag>
FMatrix mult(const FMatrix& a, const FMatrix& b) {
  assert(a.width() == b.height());   // matrix multiplication possible
  assert(a.height() == b.height());  // a and b are square
  FMatrix c{a.height(), b.width(), 0.0f};
  mult(c, a, b, Tag{});
  return c;
}

void print_equal(const char* name, const FMatrix& a, const FMatrix& b) {
  constexpr auto epsilon = 0.001f;
  bool is_equal =
      std::equal(a.begin(), a.end(), b.begin(), b.end(),
                 [epsilon](auto x, auto y) { return abs(x - y) < epsilon; });
  cout << name << ": " << boolalpha << is_equal << endl;
}

int main(int argc, char* argv[]) {
  const auto N = 1024;
  FMatrix a{N, N};
  FMatrix b{N, N};
  fill_random(a, 1, 2);
  fill_random(b, 1, 2);

  FMatrix c_single_threaded = mult<single_threaded_tag>(a, b);
  FMatrix c_multi_threaded = mult<multi_threaded_tag>(a, b);

  // cout << a << endl << b << endl;
  print_equal("MT tiled", c_single_threaded, c_multi_threaded);
  // cout << c_single_threaded << endl;
  return 0;
}
