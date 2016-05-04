#include <cassert>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "mcp.h"

template <typename T>
class queue {
 private:
  struct node {
    std::shared_ptr<node> next;
    T value;
    node(std::shared_ptr<node> next, const T &value)
        : next(next), value(value) {}
    // ~node() { std::cout << "~node" << std::endl; }
  };

 private:
  std::shared_ptr<node> head;

 private:
  node *get_last_or_nullptr() {
    if (!head) {
      return nullptr;
    }
    auto iter = head.get();
    while (iter->next != nullptr) {
      iter = iter->next.get();
    }
    return iter;
  }
  // precondition: size of list is at least 2
  node *get_last_but_one() {
    auto iter = head.get();
    while (iter->next->next != nullptr) {
      iter = iter->next.get();
    }
    return iter;
  }

 public:
  void push_back(T i) {  // TODO
    auto new_node = std::make_shared<node>(nullptr, i);
    if (head == nullptr) {
      head = new_node;
    } else {
      auto last = get_last_or_nullptr();
      last->next = new_node;
    }
  }
  void push_front(T i) {  // TODO
    auto new_head = std::make_shared<node>(head, i);
    head = new_head;
  }
  bool pop_back(T &i) {     // TODO
    if (head == nullptr) {  // list empty
      return false;
    } else if (head->next == nullptr) {  // list contains only one element
      i = head->value;
      head = nullptr;
      return true;
    } else {  // list contains at least two elements
      auto last_but_one = get_last_but_one();
      i = last_but_one->next->value;
      last_but_one->next = nullptr;
      return true;
    }
  }
  bool pop_front(T &i) {
    if (head == nullptr) {
      return false;
    }
    i = head->value;
    head = std::move(head->next);
    return true;
  }
  T *at(int i) {
    if (i < 0) return nullptr;
    auto iter = head.get();
    for (auto j = 0; j < i; ++j) {
      if (iter->next == nullptr) return nullptr;
      iter = iter->next.get();
    }
    return &iter->value;
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

void test_one_reader_one_writer() {
  using namespace std;
  using namespace std::chrono_literals;
  queue<int> ll;

  thread reader{[&] {
    for (int i = 0; i < MCP.n(); ++i) {
      int x;
      while (!ll.pop_front(x)) {
        this_thread::sleep_for(10ms);
      }
      // cout << x << endl;
    }
  }};

  thread writer{[&] {
    for (int i = 0; i < MCP.n(); ++i) {
      ll.push_front(i);
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

  test_one_reader_one_writer();

  MCP.time_stop();

  return 0;
}
