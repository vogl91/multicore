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
struct single_threaded_tiled_tag {};
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
                 single_threaded_tiled_tag) {
  const auto NN = c.height();
  const auto MM = a.width();
  const auto PP = c.width();

  const auto tile_size = sqrt(NN);
  assert(tile_size * tile_size == NN);
  for (auto ii = 0u; ii < NN; ii += tile_size) {
    const size_t N = ii + tile_size;
    for (auto jj = 0u; jj < PP; jj += tile_size) {
      const size_t P = jj + tile_size;
      for (auto kk = 0u; kk < MM; kk += tile_size) {
        const size_t M = kk + tile_size;
        tile_mult(c, a, b, ii, jj, kk, N, P, M);
      }
    }
  }
}

static void mult(FMatrix& c, const FMatrix& a, const FMatrix& b,
                 multi_threaded_tag) {
  // const auto numthreads = 4;

  const auto NN = c.height();
  const auto MM = a.width();
  const auto PP = c.width();
  const auto tile_size = cbrt(NN);
  assert(tile_size * tile_size * tile_size == NN);

  vector<thread> threads;
  for (auto ii = 0u; ii < NN; ii += tile_size*tile_size) {
    const size_t N = ii + tile_size*tile_size;
    for (auto jj = 0u; jj < PP; jj += tile_size*tile_size) {
      const size_t P = jj + tile_size*tile_size;
      for (auto kk = 0u; kk < MM; kk += tile_size*tile_size) {
        const size_t M = kk + tile_size*tile_size;
        threads.push_back(
            thread{[&]() { tile_mult(c, a, b, ii, jj, kk, N, P, M); }});
      }
    }
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
  cout << name << ": " << boolalpha << (a == b) << endl;
}

int main(int argc, char* argv[]) {
  const auto N = 64;
  FMatrix a{N, N};
  FMatrix b{N, N};
  fill_random(a, 1, 2);
  fill_random(b, 1, 2);

  FMatrix c_single_threaded = mult<single_threaded_tag>(a, b);
  FMatrix c_single_threaded_tiled = mult<single_threaded_tiled_tag>(a, b);
  FMatrix c_multi_threaded = mult<multi_threaded_tag>(a, b);

  // cout << a << endl << b << endl;
  print_equal("ST tiled", c_single_threaded, c_single_threaded_tiled);
  print_equal("MT tiled", c_single_threaded, c_multi_threaded);
  // cout << c_single_threaded << endl;
  return 0;
}
