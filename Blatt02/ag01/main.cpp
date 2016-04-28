#include <chrono>
#include <iostream>
#include <iterator>
#include <thread>
#include <vector>

#include "linkedlist.h"
#include "mcp.h"
#include "rw_lock.h"

void test_linkedlist() {
  linkedlist<int> ll;
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

int main(int argc, char* argv[]) {
  using namespace std;
  MCP.init(argc, argv);

  test_linkedlist();
  test_rw_lock();

  linkedlist<int> ll;

  MCP.time_start();

  MCP.time_stop();

  return 0;
}
