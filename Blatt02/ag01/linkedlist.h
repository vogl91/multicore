#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

#define DEBUG 1

#ifdef DEBUG
#define DBG(str) printf(str "\n")
#else
#define DBG(str)
#endif

namespace {
template <typename T>
struct node {
  node* previous;
  node* next;
  T value;
};

template <typename T>
class linkedlist_base {
 private:
  node<T>* first;
  node<T>* last;

 private:
  void init(const T& e) {
    first = new node<T>{nullptr, nullptr, e};
    last = first;
  }

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
    last->next = new node<T>{last, nullptr, e};
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
  std::cout << "]";
}
}

template <typename T>
class linkedlist : private linkedlist_base<T> {
 private:
  std::mutex m;

 public:
  linkedlist() { DBG("linkedlist()"); }
  linkedlist(const linkedlist& that) : linkedlist_base<T>{that} {
    DBG("linkedlist(const linkedlist&)");
  }
  linkedlist(linkedlist&& that) : linkedlist_base<T>{that} {
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

#endif /*LINKEDLIST_H*/
