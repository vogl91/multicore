#include <cassert>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "mcp.h"

#define DBG(str) std::cout << (str) << std::endl;

#if 0
#define RELAXED memory_order_relaxed
#define CONSUME memory_order_consume
#define AQUIRE memory_order_acquire
#define RELEASE memory_order_release
#define ACQ_REL memory_order_acq_rel
#define SEQ_CST memory_order_seq_cst
#else
#define RELAXED memory_order_seq_cst
#define CONSUME memory_order_seq_cst
#define AQUIRE memory_order_seq_cst
#define RELEASE memory_order_seq_cst
#define ACQ_REL memory_order_seq_cst
#define SEQ_CST memory_order_seq_cst
#endif

template <typename Container>
void join_all(Container &c) {
  std::for_each(c.begin(), c.end(), [](std::thread &t) {
    if (t.joinable()) t.join();
  });
}

template <typename T>
class queue {
 private:
  struct node {
    std::shared_ptr<node> next;
    T value;
    node(std::shared_ptr<node> next, const T &value)
        : next(next), value(value) {}
  };

 private:
  std::shared_ptr<node> head;

 public:
  class reference {
    std::shared_ptr<node> p;

   public:
    reference(std::shared_ptr<node> p) : p{p} {}
    T &operator*() { return p->value; }
    T *operator->() { return &p->value; }
    operator bool() { return p != nullptr; }
    bool operator==(std::nullptr_t) { return p == nullptr; }
    bool operator!=(std::nullptr_t) { return p != nullptr; }
  };

 public:
  void push_back(T i) {  // TODO
    using namespace std;
    auto new_node = make_shared<node>(nullptr, i);
    while (true) {
      auto iter = atomic_load_explicit(&head, SEQ_CST);
      auto next = shared_ptr<node>{nullptr};
      if (iter == nullptr) {
        if (atomic_compare_exchange_weak_explicit(&head, &iter, new_node,
                                                  SEQ_CST, SEQ_CST))
          break;
      } else {
        while ((next = atomic_load_explicit(&iter->next, SEQ_CST)) != nullptr) {
          iter = next;
        }
        if (atomic_compare_exchange_weak_explicit(&iter->next, &next, new_node,
                                                  SEQ_CST, SEQ_CST))
          break;
      }
    }
  }
  void push_front(T i) {
    using namespace std;
    auto new_head = make_shared<node>(atomic_load_explicit(&head, SEQ_CST), i);
    while (!atomic_compare_exchange_weak_explicit(&head, &new_head->next,
                                                  new_head, SEQ_CST, SEQ_CST))
      ;
  }
  bool pop_back(T &i) {  // TODO
    using namespace std;
    while (true) {
      auto iter = atomic_load_explicit(&head, SEQ_CST);
      auto next = shared_ptr<node>{nullptr};
      auto next_next = shared_ptr<node>{nullptr};
      if (iter == nullptr)
        return false;  // empty list
      else {
        next = atomic_load_explicit(&iter->next, SEQ_CST);
        if (next == nullptr) {  // list containing one element
          if (!atomic_compare_exchange_weak_explicit(
                  &head, &iter, shared_ptr<node>{nullptr}, SEQ_CST, SEQ_CST))
            continue;
          i = iter->value;
          return true;
        } else {  // list containing at least two elements
          while ((next_next = atomic_load_explicit(&next->next, SEQ_CST)) !=
                 nullptr) {
            iter = next;
            next = next_next;
          }
          if (!atomic_compare_exchange_weak_explicit(&iter->next, &next,
                                                     shared_ptr<node>{nullptr},
                                                     SEQ_CST, SEQ_CST))
            continue;
          i = next->value;
          return true;
        }
      }
    }
  }
  bool pop_front(T &i) {
    using namespace std;
    auto p = atomic_load_explicit(&head, SEQ_CST);
    while (p &&
           !atomic_compare_exchange_weak_explicit(&head, &p, p->next, SEQ_CST,
                                                  SEQ_CST))
      ;
    if (p == nullptr) {
      return false;
    } else {
      i = p->value;
      return true;
    }
  }
  auto at(int i) {
    using namespace std;
    if (i < 0) return reference{nullptr};
    auto iter = atomic_load_explicit(&head, SEQ_CST);
    for (auto j = 0; j < i; ++j) {
      if (iter->next == nullptr) return reference{nullptr};
      iter = iter->next;
    }
    return reference{iter};
  }
};

void test_push_pop_back() {
  queue<int> ll;
  ll.push_back(4);
  ll.push_back(3);
  ll.push_back(2);
  ll.push_back(1);
  int x;
  int results[] = {1, 2, 3, 4};
  for (int i = 0; i < 4; ++i) {
    assert(true == ll.pop_back(x));
    assert(results[i] == x);
  }
}

void test_push_pop_front() {
  queue<int> ll;
  ll.push_front(4);
  ll.push_front(3);
  ll.push_front(2);
  ll.push_front(1);
  int x;
  int results[] = {1, 2, 3, 4};
  for (int i = 0; i < 4; ++i) {
    assert(true == ll.pop_front(x));
    assert(results[i] == x);
  }
}

void test_at() {
  queue<int> ll;
  ll.push_front(2);
  ll.push_front(1);
  ll.push_front(0);

  assert(*ll.at(0) == 0);
  assert(*ll.at(1) == 1);
  assert(*ll.at(2) == 2);
  assert(ll.at(3) == nullptr);
  assert(ll.at(-1) == nullptr);
}

void test_one_reader_one_writer_front() {
  using namespace std;
  using namespace std::chrono_literals;
  queue<int> ll;
  const auto N = 100000;  // MCP.n();

  thread reader{[&] {
    for (int i = 0; i < N; ++i) {
      int x;
      while (!ll.pop_front(x)) {
        this_thread::sleep_for(10ms);
      }
      // cout << x << endl;
    }
  }};

  thread writer{[&] {
    for (int i = 0; i < N; ++i) {
      ll.push_front(i);
    }
  }};

  reader.join();
  writer.join();
}

void test_multiple_reader_writer_front() {
  using namespace std;
  using namespace std::chrono_literals;
  queue<int> ll;
  const auto N = 100000;  // MCP.n();
  const auto num_reader = 10;
  const auto num_writer = 10;

  vector<thread> threads;
  for (auto i = 0; i < num_reader; ++i) {
    threads.push_back(thread{[&] {
      for (int i = 0; i < N; ++i) {
        int x;
        while (!ll.pop_front(x)) {
          this_thread::sleep_for(10ms);
        }
        // cout << x << endl;
      }
    }});
  }

  for (auto i = 0; i < num_writer; ++i) {
    threads.push_back(thread{[&] {
      for (int i = 0; i < N; ++i) {
        ll.push_front(i);
      }
    }});
  }

  join_all(threads);
}

void test_one_reader_one_writer_back() {
  using namespace std;
  using namespace std::chrono_literals;
  queue<int> ll;
  const auto N = 1000;  // MCP.n();

  thread reader{[&] {
    for (int i = 0; i < N; ++i) {
      int x;
      while (!ll.pop_back(x)) {
        this_thread::sleep_for(10ms);
      }
      cout << "->" << x << endl;
    }
  }};

  thread writer{[&] {
    for (int i = 0; i < N; ++i) {
      // cout << i << endl;
      ll.push_back(i);
      cout << i << endl;
    }
  }};

  reader.join();
  writer.join();
}

int main(int argc, char *argv[]) {
  using namespace std;
  MCP.init(argc, argv);

  test_push_pop_back();
  test_push_pop_front();
  test_at();

  MCP.time_start();

  // test_one_reader_one_writer_front();
  test_one_reader_one_writer_back();
  // test_multiple_reader_writer_front();

  // queue<int> ll;
  // for (int i = 0; i < 100; ++i) {
  //   // cout << i << endl;
  //   ll.push_back(i);
  // }

  MCP.time_stop();

  return 0;
}
