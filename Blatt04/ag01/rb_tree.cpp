#include "rb_tree.h"
#include <iostream>

const std::shared_ptr<Tree::node> Tree::NIL =
    std::make_shared<Tree::node>(nullptr, nullptr, 0, Tree::Color::BLACK);

std::shared_ptr<Tree::node> Tree::make_node(int key) {
  return std::make_shared<node>(NIL, NIL, key, Tree::Color::RED);
}

Tree::Tree() : root{Tree::NIL} {}

bool Tree::insert(int key) {
  if (root->is_nil()) {
    root = make_node(key);
    return true;
  }
  auto iter = root.get();
  while (true) {
    if (key < iter->key) {
      if (iter->left->is_nil()) {
        iter->left = make_node(key);
        return true;
      } else {
        iter = iter->left.get();
      }
    } else if (key > iter->key) {
      if (iter->right->is_nil()) {
        iter->right = make_node(key);
        return true;
      } else {
        iter = iter->right.get();
      }
    } else {  // key == iter->key
      return false;
    }
  }
}
bool Tree::search(int key, Tree::node& n) {
  auto iter = root.get();
  while (true) {
    if (iter->is_nil()) {
      return false;
    } else if (key < iter->key) {
      iter = iter->left.get();
    } else if (key > iter->key) {
      iter = iter->right.get();
    } else {  // key == iter->key
      n = *iter;
      return true;
    }
  }
}
bool Tree::deleteValue(int key) {
  // TODO
  return false;
}

void Tree::for_each(std::function<void(const Tree::node&)> func) const {
  for_each(func, root);
}

void Tree::for_each(std::function<void(const Tree::node&)> func,
                    std::shared_ptr<Tree::node> root) const {
  if (root->is_nil()) return;
  for_each(func, root->left);
  func(*root);
  for_each(func, root->right);
}

bool Tree::assert_children_black_if_node_red() const {
  bool b = true;
  for_each([&](const node& n) {
    if (n.color == Color::RED) {
      b &= n.left->color == Color::BLACK && n.right->color == Color::BLACK;
    }
  });
  return b;
}

bool Tree::assert_each_path_contains_same_number_of_black_nodes() const {
  bool b = true;
  assert_each_path_contains_same_number_of_black_nodes(*root, b);
  return b;
}

size_t Tree::assert_each_path_contains_same_number_of_black_nodes(
    const node& n, bool& b) const {
  if (n.is_nil()) return 1;
  auto l = assert_each_path_contains_same_number_of_black_nodes(*n.left, b);
  auto r = assert_each_path_contains_same_number_of_black_nodes(*n.right, b);
  b &= l == r;
  return l + r + (n.color == Color::BLACK ? 1 : 0);
}

bool Tree::test_assertions() const {
  return assert_children_black_if_node_red() &&
         assert_each_path_contains_same_number_of_black_nodes();
}

std::ostream& operator<<(std::ostream& os, const Tree& t) {
  os << "[ ";
  t.for_each([&os](const Tree::node& n) {
    os << n.key << "(" << (n.color == Tree::Color::BLACK ? "B" : "R") << ")"
       << " ";
  });
  os << "]";
  return os;
}

int main(int argc, char* argv[]) {
  using namespace std;
  cout << "Red Black Tree" << endl;
  Tree t;
  for (int i = 0; i < 10; ++i) {
    t.insert(i);
  }
  t.insert(1);
  cout << t << endl;
  cout << t.assert_children_black_if_node_red() << endl;
  cout << t.assert_each_path_contains_same_number_of_black_nodes() << endl;
  return 0;
}
