#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

#include "rw_lock.h"

#define DEBUG 1

#ifdef DEBUG
#define DBG(str) printf(str "\n")
#else
#define DBG(str)
#endif

namespace {

template <typename T>
class linkedlist_base {
 protected:
  struct node {
    node* previous;
    node* next;
    T value;
  };

 protected:
  node* first;
  node* last;

 protected:
  void init(const T& e);

 public:
  linkedlist_base();
  linkedlist_base(const linkedlist_base& that);
  linkedlist_base(linkedlist_base&& that);
  ~linkedlist_base();

  void push_back(T e);
  T at(int index);
  T pop_back();
  void print();
};
template <typename T>
void linkedlist_base<T>::init(const T& e) {
  first = new node{nullptr, nullptr, e};
  last = first;
}

template <typename T>
linkedlist_base<T>::linkedlist_base() : first(nullptr), last(nullptr) {
  DBG("linkedlist_base()");
}

template <typename T>
linkedlist_base<T>::linkedlist_base(const linkedlist_base& that)
    : linkedlist_base() {
  DBG("linkedlist_base(const linkedlist_base&)");
  for (auto iter = that.first; iter != nullptr; iter = iter->next) {
    push_back(iter->value);
  }
}

template <typename T>
linkedlist_base<T>::linkedlist_base(linkedlist_base&& that)
    : first(that.first), last(that.last) {
  DBG("linkedlist_base(linkedlist_base&&)");
  that.first = nullptr;
  that.last = nullptr;
}

template <typename T>
linkedlist_base<T>::~linkedlist_base() {
  DBG("~linkedlist_base()");
  auto iter = first;
  while (iter != nullptr) {
    auto tmp = iter->next;
    delete iter;
    iter = tmp;
  }
}
template <typename T>
void linkedlist_base<T>::push_back(T e) {
  // list empty?
  if (last == nullptr) {
    init(e);
  } else {
    last->next = new node{last, nullptr, e};
    last = last->next;
  }
}
template <typename T>
T linkedlist_base<T>::at(int index) {
  assert(first != nullptr);
  auto iter = first;
  for (int i = 0; i < index; ++i) {
    iter = iter->next;
    assert(iter != nullptr);
  }
  return iter->value;
}
template <typename T>
T linkedlist_base<T>::pop_back() {
  assert(last != nullptr);
  T value = last->value;
  if (first == last) {
    delete last;
    first = nullptr;
    last = nullptr;
  } else {
    last = last->previous;
    delete last->next;
    last->next = nullptr;
  }
  return value;
}
template <typename T>
void linkedlist_base<T>::print() {
  std::cout << "[ ";
  for (auto iter = first; iter != nullptr; iter = iter->next) {
    std::cout << iter->value << " ";
  }
  std::cout << "]" << std::endl;
}
}

/***** PUBLIC INTERFACE *****/

struct mutex_locking_tag {};
struct reader_writer_locking_tag {};
struct fine_grained_locking_tag {};

template <typename T, typename LockingType = mutex_locking_tag>
class linkedlist;

/**
 * default implementation using mutex to protect every operation
 * TODO: constructors/destructors are not protected
 */
template <typename T>
class linkedlist<T, mutex_locking_tag> : private linkedlist_base<T> {
 private:
  std::mutex m;

 public:
  linkedlist() { DBG("linkedlist()"); }
  linkedlist(const linkedlist& that) : linkedlist_base<T>{that} {
    DBG("linkedlist(const linkedlist&)");
  }
  linkedlist(linkedlist&& that) : linkedlist_base<T>{std::move(that)} {
    DBG("linkedlist(linkedlist&&)");
  }
  ~linkedlist() { DBG("~linkedlist()"); }

  void push_back(T e) {
    std::lock_guard<std::mutex> lock{m};
    linkedlist_base<T>::push_back(std::move(e));
  }
  T at(int index) {
    std::lock_guard<std::mutex> lock{m};
    return linkedlist_base<T>::at(index);
  }
  T pop_back() {
    std::lock_guard<std::mutex> lock{m};
    return linkedlist_base<T>::pop_back();
  }
  void print() {
    std::lock_guard<std::mutex> lock{m};
    linkedlist_base<T>::print();
  }
};

/**
 * reader-writer locking
 * writer locks: push_back, pop_back()
 * reader_locks: at, print
 */
template <typename T>
class linkedlist<T, reader_writer_locking_tag> : private linkedlist_base<T> {
 private:
  rw_lock rw;

 public:
  linkedlist() { DBG("linkedlist()"); }
  linkedlist(const linkedlist& that) : linkedlist_base<T>{that} {
    DBG("linkedlist(const linkedlist&)");
  }
  linkedlist(linkedlist&& that) : linkedlist_base<T>{std::move(that)} {
    DBG("linkedlist(linkedlist&&)");
  }
  ~linkedlist() { DBG("~linkedlist()"); }

  void push_back(T e) {
    write_lock_guard lock{rw};
    linkedlist_base<T>::push_back(std::move(e));
  }
  T at(int index) {
    read_lock_guard lock{rw};
    return linkedlist_base<T>::at(index);
  }
  T pop_back() {
    write_lock_guard lock{rw};
    return linkedlist_base<T>::pop_back();
  }
  void print() {
    read_lock_guard lock{rw};
    linkedlist_base<T>::print();
  }
};

/**
 * fine-grained locking
 */

template <typename T>
class linkedlist<T, fine_grained_locking_tag> {
 private:
  struct node {
    node* previous;
    node* next;
    T value;
    rw_lock rw;
  };

 private:
  node* first;
  node* last;
  rw_lock rw;

 public:
  linkedlist() : first(nullptr), last(nullptr) { DBG("linkedlist()"); }
  linkedlist(const linkedlist& that) : linkedlist{} {
    DBG("linkedlist(const linkedlist&)");
    for (auto iter = that.first; iter != nullptr; iter = iter->next) {
      push_back(iter->value);
    }
  }
  linkedlist(linkedlist&& that) : first(that.first), last(that.last) {
    DBG("linkedlist(linkedlist&&)");
    that.first = nullptr;
    that.last = nullptr;
  }
  ~linkedlist() {
    DBG("~linkedlist()");
    auto iter = first;
    while (iter != nullptr) {
      auto tmp = iter->next;
      delete iter;
      iter = tmp;
    }
  }

  void push_back(T e) {
    // list empty?
    if (last == nullptr) {
      rw.lock_write();
      first = new node{nullptr, nullptr, e};
      last = first;
      rw.unlock_write();
    } else {
      last->rw.lock_write();
      last->next = new node{last, nullptr, e};
      last = last->next;
      last->previous->rw.unlock_write();
    }
  }
  T at(int index) {
    assert(first != nullptr);
    auto iter = first;
    for (int i = 0; i < index; ++i) {
      iter->rw.lock_read();
      iter = iter->next;
      iter->rw.unlock_read();
      assert(iter != nullptr);
    }
    return iter->value;
  }
  T pop_back() {
    assert(last != nullptr);
    last->rw.lock_write();
    T value = last->value;
    if (first == last) {
      last->rw.unlock_write();
      delete last;
      first = nullptr;
      last = nullptr;
    } else {
      last->previous->rw.lock_write();
      last = last->previous;
      last->next->rw.unlock_write();
      delete last->next;
      last->next = nullptr;
      last->rw.unlock_write();
    }
    return value;
  }
  void print() {
    std::cout << "[ ";
    for (auto iter = first; iter != nullptr; iter = iter->next) {
      iter->rw.lock_read();
      std::cout << iter->value << " ";
      iter->rw.unlock_read();
    }
    std::cout << "]" << std::endl;
  }
};

#endif /*LINKEDLIST_H*/
