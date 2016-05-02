#include <cassert>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <list>
#include <random>
#include <thread>
#include <vector>

#include "mcp.h"

template <typename Iter>
void print_range(Iter first, Iter last) {
  auto& os = std::cout;

  os << "[ ";
  for (auto iter = first; iter != last; ++iter) {
    if (iter != first) {
      os << ",";
    }
    os << *iter << " ";
  }
  os << "]" << std::endl;
}

template <typename T>
void print_list(const std::list<T>& list) {
  print_range(list.begin(), list.end());
}

/**
 * find middle of range [first,last) and return iterator to it
 * Iter shall model BidirectionalIterator
 * if last is before first the behaviour is undefined
 */
template <typename Iter>
Iter find_middle(Iter first, Iter last) {
  Iter fast = first;
  Iter slow = first;
  while (fast != last) {
    ++fast;
    if (fast != last) {
      ++fast;
      ++slow;
    }
  }
  return slow;
}

/**
 * split list in middle returning the lower half in a new list
 * if number of elements is odd than the middle element remains in old list
 */
template <typename T>
std::list<T> split_in_middle(std::list<T>& list) {
  std::list<T> other;
  other.splice(other.begin(), list, list.begin(),
               find_middle(list.begin(), list.end()));
  return other;
}

template <typename T>
std::list<T> mergesort(std::list<T> ll) {
  using namespace std;
  if (ll.size() < 2) {
    return ll;
  }
  list<T> other = split_in_middle(ll);

  ll = mergesort(ll);
  other = mergesort(other);

  other.merge(ll);
  return other;
}

template <typename T>
std::list<T> mergesort_parallel(std::list<T> ll) {
  using namespace std;
  if (ll.size() < 2) {
    return ll;
  }
  list<T> other = split_in_middle(ll);
  auto future_other = async(launch::async, mergesort_parallel<T>, other);
  // auto future_other = async(mergesort_parallel<T>, other);

  ll = mergesort(ll);
  other = future_other.get();

  other.merge(ll);
  return other;
}

template <typename T>
std::list<T> mergesort_parallel2(std::list<T> ll,
                                 std::atomic<int>* remaining_threads) {
  using namespace std;
  if (ll.size() < 2) {
    return ll;
  }
  list<T> other = split_in_middle(ll);

  if (--*remaining_threads >= 0) {
    auto future_other =
        async(launch::async, mergesort_parallel2<T>, other, remaining_threads);
    // auto future_other = async(mergesort_parallel<T>, other);
    ll = mergesort_parallel2(ll, remaining_threads);
    other = future_other.get();
  } else {
    ll = mergesort(ll);
    other = mergesort(other);
  }

  other.merge(ll);
  return other;
}

template <typename T>
std::list<T> mergesort_parallel2(std::list<T>& ll, int number_of_threads) {
  std::atomic<int> remaining_threads;
  remaining_threads = number_of_threads;
  return mergesort_parallel2(ll, &remaining_threads);
}

template <typename Iter>
void mergesort2(Iter first, Iter last) {
  using namespace std;
  if (first == last || ++Iter{first} == last) {
    return;
  }
  auto middle = find_middle(first, last);

  mergesort2(first, middle);
  mergesort2(middle, last);

  std::inplace_merge(first, middle, last);
}

template <typename Iter>
void mergesort2_parallel(Iter first, Iter last,
                         std::atomic<int>* remaining_threads) {
  using namespace std;
  if (first == last || ++Iter{first} == last) {
    return;
  }
  auto middle = find_middle(first, last);

  if (--*remaining_threads >= 0) {
    auto fut = async(launch::async, mergesort2_parallel<Iter>, first, middle,
                     remaining_threads);
    mergesort2_parallel(middle, last, remaining_threads);
    fut.wait();
  } else {
    mergesort2(first, middle);
    mergesort2(middle, last);
  }

  std::inplace_merge(first, middle, last);
}

template <typename Iter>
void mergesort2_parallel(Iter first, Iter last, int number_of_threads) {
  std::atomic<int> remaining_threads;
  remaining_threads = number_of_threads;
  mergesort2_parallel(first, last, &remaining_threads);
}

/*======== UNIT TESTS ========*/
void test_find_middle() {
  using namespace std;

  const auto assert_equal = [](const list<int>& l, int result) {
    assert(*find_middle(l.begin(), l.end()) == result);
  };

  list<int> l;
  assert(find_middle(l.begin(), l.end()) == l.begin());

  assert_equal({1}, 1);
  assert_equal({1, 2}, 2);
  assert_equal({1, 2, 3}, 2);
  assert_equal({1, 2, 3, 4}, 3);
  assert_equal({1, 2, 3, 4, 5}, 3);
}

void test_split_in_midle() {
  using namespace std;
  list<int> l;
  auto other = split_in_middle(l);
  assert(l.empty());
  assert(other.empty());

  l = {1};
  other = split_in_middle(l);
  assert(other == list<int>{});
  assert(l == list<int>{1});

  l = {1, 2};
  other = split_in_middle(l);
  assert(other == list<int>{1});
  assert(l == list<int>{2});

  l = {1, 2, 3};
  other = split_in_middle(l);
  assert((other == list<int>{1}));
  assert((l == list<int>{2, 3}));

  l = {1, 2, 3, 4};
  other = split_in_middle(l);
  assert((other == list<int>{1, 2}));
  assert((l == list<int>{3, 4}));

  l = {1, 2, 3, 4, 5};
  other = split_in_middle(l);
  assert((other == list<int>{1, 2}));
  assert((l == list<int>{3, 4, 5}));
}

void test_mergesort() {
  using namespace std;

  assert(mergesort(list<int>{}) == list<int>{});
  assert(mergesort(list<int>{1}) == list<int>{1});
  assert((mergesort(list<int>{2, 1}) == list<int>{1, 2}));
  assert((mergesort(list<int>{2, 3, 1}) == list<int>{1, 2, 3}));

  list<int> l{1, 6, 8, 5, 3, 2, 6, 9, 4, 6, 2, 9, 6,
              7, 2, 1, 1, 3, 4, 6, 9, 3, 9, 2, 0};
  auto sorted = mergesort(l);
  assert(std::is_sorted(sorted.begin(), sorted.end()));
}
/*======== MAIN ========*/
int main(int argc, char* argv[]) {
  using namespace std;

  MCP.init(argc, argv);

  test_find_middle();
  test_split_in_midle();
  test_mergesort();

  default_random_engine generator;
  uniform_int_distribution<int> distribution(0, 9);
  auto next_int = std::bind(distribution, generator);

  cout << "*========* using " << MCP.n() << " samples *========*" << endl;
  list<int> l;
  for (int i = 0; i < MCP.n(); ++i) {
    l.push_back(next_int());
  }

  auto test = [&](const char* name, auto func) {
    auto copy_of_l = l;
    // print_list(copy_of_l);
    cout << "sorting with " << name << "..." << endl;
    MCP.time_start();
    func(copy_of_l);
    MCP.time_stop();

    // cout << "testing if sorted..." << endl;
    // print_list(copy_of_l);
    assert(std::is_sorted(copy_of_l.begin(), copy_of_l.end()));
    MCP.time_print();
  };

  vector<int> v{l.begin(), l.end()};
  cout << "sorting vector ..." << endl;
  MCP.time_start();
  sort(v.begin(), v.end());
  MCP.time_stop();
  assert(std::is_sorted(v.begin(), v.end()));
  MCP.time_print();

  vector<int> v2;
  cout << "sorting by copying vector ..." << endl;
  list<int> copy_of_l;
  MCP.time_start();
  v2.assign(l.begin(), l.end());
  sort(v.begin(), v.end());
  copy_of_l.assign(v.begin(), v.end());
  MCP.time_stop();
  assert(std::is_sorted(copy_of_l.begin(), copy_of_l.end()));
  MCP.time_print();

  test("list::sort", [](auto& l) { l.sort(); });

  test("mergesort", [](auto& l) { l = mergesort(l); });

  test("mergesort_parallel", [](auto& l) { l = mergesort_parallel(l); });

  test("mergesort_parallel2, 2 threads",
       [](auto& l) { l = mergesort_parallel2(l, 2); });
  test("mergesort_parallel2, 4 threads",
       [](auto& l) { l = mergesort_parallel2(l, 4); });
  test("mergesort_parallel2, 8 threads",
       [](auto& l) { l = mergesort_parallel2(l, 8); });

  test("mergesort2", [](auto& l) { mergesort2(l.begin(), l.end()); });

  test("mergesort2_parallel, 2 threads",
       [](auto& l) { mergesort2_parallel(l.begin(), l.end(), 2); });
  test("mergesort2_parallel, 4 threads",
       [](auto& l) { mergesort2_parallel(l.begin(), l.end(), 4); });
  test("mergesort2_parallel, 8 threads",
       [](auto& l) { mergesort2_parallel(l.begin(), l.end(), 8); });
  return 0;
}
