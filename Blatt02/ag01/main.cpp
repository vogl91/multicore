#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <random>
#include <thread>
#include <typeinfo>
#include <vector>

#include "linkedlist.h"
#include "mcp.h"
#include "rw_lock.h"

// using list_t = linkedlist<int, mutex_locking_tag>;
using list_t = linkedlist<int, reader_writer_locking_tag>;
// using list_t = linkedlist<int, fine_grained_locking_tag>;

template <typename Container>
void join_all(Container& c) {
  std::for_each(c.begin(), c.end(), [](std::thread& t) {
    if (t.joinable()) t.join();
  });
}

struct random_int_generator {
 private:
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution;

 public:
  random_int_generator(int min, int max) : distribution{min, max} {}
  int operator()() { return distribution(generator); }
};

void test_linkedlist() {
  list_t ll;
  ll.push_back(1);
  assert(ll.at(0) == 1);

  ll.push_back(2);
  assert(ll.at(0) == 1);
  assert(ll.at(1) == 2);

  assert(ll.pop_back() == 2);
  assert(ll.at(0) == 1);
  assert(ll.pop_back() == 1);
}

void lock_wait_unlock(rw_lock* lock, bool reader) {
  using namespace std::chrono_literals;
  if (reader)
    lock->lock_read();
  else
    lock->lock_write();
  std::cout << (reader ? "R" : "W") << " in" << std::endl;
  std::this_thread::sleep_for(1s);
  std::cout << (reader ? "R" : "W") << " out" << std::endl;
  if (reader)
    lock->unlock_read();
  else
    lock->unlock_write();
}

void test_rw_lock() {
  using namespace std;
  rw_lock lock;
  {
    auto t1 = thread{lock_wait_unlock, &lock, true};
    auto t2 = thread{lock_wait_unlock, &lock, true};
    auto t3 = thread{lock_wait_unlock, &lock, false};
    t1.join();
    t2.join();
    t3.join();
  }
  std::this_thread::sleep_for(1s);
  std::cout << std::endl;
  {
    auto t1 = thread{lock_wait_unlock, &lock, false};
    auto t2 = thread{lock_wait_unlock, &lock, true};
    auto t3 = thread{lock_wait_unlock, &lock, true};
    t1.join();
    t2.join();
    t3.join();
  }
}

list_t generate_random_list() {
  list_t ll;
  random_int_generator random_int{1, 100};
  for (int i = 0; i < MCP.n(); ++i) {
    ll.push_back(random_int());
  }
  return ll;
}

void test_read_only(list_t& ll) {
  using namespace std;
  cout << "test_read_only" << endl;
  cout << typeid(list_t).name() << endl;
  vector<thread> threads;
  for (int i = 0; i < MCP.t(); ++i) {
    threads.emplace_back([&] {
      for (int i = 0; i < MCP.n() / MCP.t(); ++i) {
        ll.at(MCP.n()-1);
      }
    });
  }
  join_all(threads);
}
void test_write_only(list_t& ll) {
  using namespace std;
  cout << "test_read_only" << endl;
  cout << typeid(list_t).name() << endl;
  vector<thread> threads;
  for (int i = 0; i < MCP.t(); ++i) {
    threads.emplace_back([&] {
      for (int i = 0; i < MCP.n() / MCP.t(); ++i) {
        ll.pop_back();
      }
    });
  }
  join_all(threads);
}

void test_multiple_read_one_write(list_t& ll) {
  using namespace std;
  cout << "test_multiple_read_one_write" << endl;
  cout << typeid(list_t).name() << endl;

  vector<thread> threads;
  for (int i = 0; i < MCP.t() - 1; ++i) {
    threads.emplace_back([&] {
      cout << "reader" << endl;
      for (int i = 0; i < MCP.n() / (MCP.t() - 1); ++i) {
        ll.at(MCP.n()-1);
      }
    });
  }
  threads.emplace_back([&] {
    cout << "writer" << endl;
    for (int i = 0; i < MCP.n(); ++i) {
      ll.push_back(1);
    }
  });

  join_all(threads);
}

void test_multiple_read_one_slow_write(list_t& ll) {
  using namespace std;
  cout << "test_multiple_read_one_slow_write" << endl;
  cout << typeid(list_t).name() << endl;

  vector<thread> threads;
  for (int i = 0; i < MCP.t() - 1; ++i) {
    threads.emplace_back([&] {
      cout << "reader" << endl;
      for (int i = 0; i < MCP.n() / (MCP.t() - 1); ++i) {
        ll.at(MCP.n()-1);
      }
    });
  }
  threads.emplace_back([&] {
    cout << "writer" << endl;
    while (threads[0].joinable()) {
      this_thread::sleep_for(100ms);
      ll.push_back(1);
    }
  });

  join_all(threads);
}

void test_multiple_read_multiple_write(list_t& ll) {
  using namespace std;
  cout << "test_multiple_read_multiple_slow_write" << endl;
  cout << typeid(list_t).name() << endl;

  vector<thread> threads;
  for (int i = 0; i < MCP.t() / 2; ++i) {
    threads.emplace_back([&] {
      cout << "reader" << endl;
      for (int i = 0; i < MCP.n() / (MCP.t() - 1); ++i) {
        ll.at(MCP.n()-1);
      }
    });
  }
  for (int i = 0; i < MCP.t() / 2; ++i) {
    threads.emplace_back([&] {
      cout << "writer" << endl;
      while (threads[0].joinable()) {
        ll.push_back(1);
      }
    });
  }

  join_all(threads);
}

int main(int argc, char* argv[]) {
  using namespace std;
  MCP.init(argc, argv);

  // test_linkedlist();
  // test_rw_lock();

  list_t ll = generate_random_list();

  MCP.time_start();

  // test_read_only(ll);
  // test_write_only(ll);
  // test_multiple_read_one_write(ll);
  // test_multiple_read_one_slow_write(ll);
  test_multiple_read_multiple_write(ll);

  MCP.time_stop();

  return 0;
}
