#include <cassert>
#include <climits>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <iomanip>
#include <iostream>
using namespace std;

#define DBG(str) cout << str << endl;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

constexpr long long pow2(long long x) { return x == 0 ? 1 : 2 * pow2(x - 1); }
constexpr bool is_divisible_by(long long by, long long x) {
  return x % by == 0;
}

void radix_sort(int *a, int length) {
  constexpr int digits  = 8; // empirical best value
  constexpr int radix   = pow2(digits);
  constexpr int numbits = sizeof(int) * CHAR_BIT;
  static_assert(is_divisible_by(digits, numbits),
                "digits must be a dividend of numbits");
  int indizes[radix];
  int *t1 = a;
  int *t2 = new int[length];

  for (int r = 0; r < numbits; r += digits) {
    fill(begin(indizes), end(indizes), 0);
    int mask = (radix - 1) << r;

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

template <typename T> void print(ostream &os, const T *a, size_t length) {
  os << "[ ";
  for (size_t i = 0u; i < length; ++i) {
    os << a[i] << " ";
  }
  os << "]" << endl;
}

int main(int argc, char const *argv[]) {
  srand(time(nullptr));
  constexpr int size = 100'000'000;
  // constexpr int size = 10;
  int *a = new int[size];
  generate(a, a + size, rand);
  // print(cout, a, size);
  radix_sort(a, size);
  // print(cout, a, size);
  assert(is_sorted(a, a + size));
  delete[] a;
  return 0;
}
