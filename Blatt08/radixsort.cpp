#include <cassert>
#include <climits>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
using namespace std;

#define DBG(str) cout << str << endl;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

template <typename T>
void print(ostream &os, const T *a, size_t length) {
  os << "[ ";
  for (size_t i = 0u; i < length; ++i) {
    os << a[i] << " ";
  }
  os << "]" << endl;
}

constexpr unsigned pow2(unsigned x) { return x == 0 ? 1 : 2 * pow2(x - 1); }
constexpr bool is_divisible_by(unsigned by, unsigned x) { return x % by == 0; }

void radix_sort(unsigned *a, unsigned length) {
  constexpr int digits = 16;  // empirical best value
  constexpr int radix = pow2(digits);
  constexpr int numbits = sizeof(*a) * CHAR_BIT;
  static_assert(is_divisible_by(digits, numbits),
                "digits must be a dividend of numbits");
  unsigned indizes[radix];
  unsigned *t1 = a;
  unsigned *t2 = new unsigned[length];

  for (int r = 0; r < numbits; r += digits) {
    fill(begin(indizes), end(indizes), 0);
    unsigned mask = (radix - 1) << r;

#define INDEX(t, i) ((mask & (t)[(i)]) >> r)

    for (int i = 0; i < length; ++i) {
      ++indizes[INDEX(t1, i)];
    }
    for (int i = 1; i < ARRAY_SIZE(indizes); ++i) {
      indizes[i] += indizes[i - 1];
    }
    for (int i = length - 1; i >= 0; --i) {
      t2[--indizes[INDEX(t1, i)]] = t1[i];
    }
    swap(t1, t2);
  }
  if (a != t1) {
    copy(t1, t1 + length, a);
  }

  delete[] t2;
#undef INDEX
}

#define IS_ZERO(a, i, bit) (((1 << bit) & (~a[i])) >> bit)
unsigned prefix_sum(unsigned *a, unsigned *scan, unsigned length,
                    unsigned bit) {
  scan[0] = 0;
  unsigned zeros = 0;
  for (int i = 1; i < length; ++i) {
    auto is_zero = IS_ZERO(a, i - 1, bit);
    zeros += is_zero;
    scan[i] = scan[i - 1] + is_zero;
  }
  zeros += IS_ZERO(a, length - 1, bit);
  return zeros;
}

void radix_sort2(unsigned *a, unsigned length) {
  constexpr int numbits = sizeof(*a) * CHAR_BIT;
  unsigned *tmp = new unsigned[length];
  unsigned *scan = new unsigned[length];
  int bit;
  for (bit = 0; bit < numbits; ++bit) {
    auto zeros = prefix_sum(a, scan, length, bit);
    // if (zeros == length) {
    //   break;
    // }
    for (int i = 0; i < length; ++i) {
      tmp[IS_ZERO(a, i, bit) ? (scan[i]) : (i - scan[i] + zeros)] = a[i];
    }
    swap(a, tmp);
  }
  if (bit % 2 != 0) swap(a, tmp);
  delete[] tmp;
  delete[] scan;
}
#undef IS_ZERO

int main(int argc, char const *argv[]) {
  srand(time(nullptr));
  constexpr int size = 10'000'000;
  // constexpr int size = 16;
  unsigned *a = new unsigned[size];
  generate(a, a + size, []() { return rand() % (1 << 30); });
  // a[0] = 0b11;
  // a[1] = 0b00;
  // a[2] = 0b10;
  // a[3] = 0b01;
  // print(cout, a, size);
  // std::sort(a,a+size);
  // radix_sort(a, size);
  radix_sort2(a, size);
  // print(cout, a, size);
  // assert(is_sorted(a, a + size));
  delete[] a;
  return 0;
}
