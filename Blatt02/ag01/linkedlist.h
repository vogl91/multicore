#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

namespace {
template <typename T>
struct node {
  node* previous;
  node* next;
  T value;
};
}

template <typename T>
class linkedlist {
 private:
  node<T>* first;
  node<T>* last;

  std::mutex m;

 private:
  void init(const T& e) {
    first = new node<T>{nullptr, nullptr, e};
    last = first;
  }

 public:
  linkedlist();
  linkedlist(const linkedlist& that);
  linkedlist(linkedlist&& that);
  ~linkedlist();

  void push_back(T e);
  T at(int index);
  T pop_back();
  void print();
};

template <typename T>
linkedlist<T>::linkedlist() : first(nullptr), last(nullptr) {}

template <typename T>
linkedlist<T>::linkedlist(const linkedlist& that) : linkedlist() {
  for (auto iter = that.first; iter != nullptr; iter = iter.next) {
    push_back(iter->value);
  }
}

template <typename T>
linkedlist<T>::linkedlist(linkedlist&& that)
    : first(that.first), last(that.last), m(std::move(that.m)) {
  that.first = nullptr;
  that.last = nullptr;
}

template <typename T>
linkedlist<T>::~linkedlist() {
  auto iter = first;
  while (first != nullptr) {
    auto tmp = iter->next;
    delete iter;
    iter = tmp;
  }
}
template <typename T>
void linkedlist<T>::push_back(T e) {
  std::lock_guard<std::mutex> lock{m};
  // list empty
  if (last == nullptr) {
    init(e);
  } else {
    last->next = new node<T>{last, nullptr, e};
    last = last->next;
  }
}
template <typename T>
T linkedlist<T>::at(int index) {
  std::lock_guard<std::mutex> lock{m};
  assert(first != nullptr);
  auto iter = first;
  for (int i = 0; i < index; ++i) {
    iter = iter->next;
    assert(iter != nullptr);
  }
  return iter->value;
}
template <typename T>
T linkedlist<T>::pop_back() {
  std::lock_guard<std::mutex> lock{m};
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
void linkedlist<T>::print() {
  std::lock_guard<std::mutex> lock{m};
  std::cout << "[ ";
  for (auto iter = first; iter != nullptr; iter = iter->next) {
    std::cout << iter->value << " ";
  }
  std::cout << "]";
}

#endif
