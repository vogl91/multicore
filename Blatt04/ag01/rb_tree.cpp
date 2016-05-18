#include <cassert>

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <thread>
#include <vector>

#include "rb_tree.h"

void debug_print(const rb_tree& t, bool print_color);

template <typename Container>
void join_all(Container& c) {
  std::for_each(c.begin(), c.end(), [](std::thread& t) {
    if (t.joinable()) t.join();
  });
}

/*========* node *========*/
rb_tree::node::~node() {
  if (is_nil()) return;
  delete left;
  delete right;
}

bool rb_tree::node::is_nil() const {
  return left == nullptr && right == nullptr;
}

rb_tree::node* rb_tree::node::grandparent() const {
  if (parent != nullptr)
    return parent->parent;
  else
    return nullptr;
}

rb_tree::node* rb_tree::node::uncle() const {
  node* g = grandparent();
  if (g == nullptr) return nullptr;  // No grandparent means no uncle
  if (parent == g->left)
    return g->right;
  else
    return g->left;
}

rb_tree::node* rb_tree::node::sibling() const {
  if (parent == nullptr) return nullptr;
  if (this == parent->left)
    return parent->right;
  else
    return parent->left;
}

/*========* rb_tree INTERFACE *========*/

rb_tree::rb_tree() : root{make_nil(nullptr)} {}

rb_tree::~rb_tree() { delete root; }

bool rb_tree::insert(int key) {
  RTMLock lock{lockElision};

  node* inserted_node = nullptr;
  if (!insert(key, inserted_node))
    return false;
  else
    return insert_case1(inserted_node), true;
}
bool rb_tree::search(rb_tree::node& n, int key) {
  RTMLock lock{lockElision};
  auto result = search(key);
  if (result == nullptr)
    return false;
  else
    return n = *result, true;
}
bool rb_tree::deleteValue(int key) {
  RTMLock lock{lockElision};
  node* n = search(key);
  if (n == nullptr) {
    return false;
  } else {
    node* succ = find_successor(n);
    swap_nodes(n, succ);
    return deleteValue(succ), true;
  }
}

void rb_tree::for_each(std::function<void(const rb_tree::node&)> func) {
  infix_traversal(func, root);
}
void rb_tree::prefix_traversal(std::function<void(const rb_tree::node&)> func) {
  RTMLock lock{lockElision};
  prefix_traversal(func, root);
}
void rb_tree::infix_traversal(std::function<void(const rb_tree::node&)> func) {
  RTMLock lock{lockElision};
  infix_traversal(func, root);
}
void rb_tree::postfix_traversal(
    std::function<void(const rb_tree::node&)> func) {
  RTMLock lock{lockElision};
  postfix_traversal(func, root);
}
rb_tree::node* rb_tree::get_root() { return root; }

/*========* rb_tree PRIVATE helper *========*/
rb_tree::node* rb_tree::make_nil(rb_tree::node* parent) {
  return new node{nullptr, nullptr, parent, 0, color::BLACK};
}
rb_tree::node* rb_tree::make_node(rb_tree::node* parent, int key) {
  node* new_node = new node;
  new_node->left = make_nil(new_node);
  new_node->right = make_nil(new_node);
  new_node->parent = parent;
  new_node->key = key;
  new_node->color_ = color::RED;

  return new_node;
}

void rb_tree::rotate_left(rb_tree::node* n) {
  /*
   *   (p)   |   (p)
   *    n    |    a
   *  1   a  |  n   2
   *     b 2 | 1 b
   */
  assert(!n->is_nil());
  assert(!n->right->is_nil());
  node* const a = n->right;
  node* const b = a->left;
  node* const p = n->parent;
  a->left = n, n->parent = a;
  n->right = b, b->parent = n;
  if (p == nullptr) {
    // n is root
    a->parent = nullptr;
    root = a;
  } else {
    a->parent = p;
    if (p->left == n)
      p->left = a;
    else
      p->right = a;
  }
}
void rb_tree::rotate_right(rb_tree::node* n) {
  /*
   *   (p)   |   (p)
   *    n    |    a
   *  a   1  |  2   n
   * 2 b     |     b 2
   */
  assert(!n->is_nil());
  assert(!n->left->is_nil());
  node* const a = n->left;
  node* const b = a->right;
  node* const p = n->parent;
  a->right = n, n->parent = a;
  n->left = b, b->parent = n;
  if (p == nullptr) {
    // n is root
    a->parent = nullptr;
    root = a;
  } else {
    a->parent = p;
    if (p->left == n)
      p->left = a;
    else
      p->right = a;
  }
}

void rb_tree::swap_nodes(rb_tree::node* n, rb_tree::node* m) {
  // assert(!n->is_nil());
  // assert(!m->is_nil());
  int tmp = n->key;
  n->key = m->key;
  m->key = tmp;
}
rb_tree::node* rb_tree::find_successor(const rb_tree::node* n) const {
  if (n->right->is_nil()) {
    auto iter = n;
    while (iter->parent && iter->parent->right == iter) {
      iter = iter->parent;
    }
    if (iter->parent && iter->parent->left == iter) {
      return iter->parent;
    } else {
      return nullptr;
    }
  } else {
    auto iter = n->right;
    while (!iter->left->is_nil()) {
      iter = iter->left;
    }
    return iter;
  }
}

/*========* rb_tree PRIVATE search *========*/
rb_tree::node* rb_tree::search(int key) const {
  auto iter = root;
  while (true) {
    if (iter->is_nil()) {
      return nullptr;
    } else if (key < iter->key) {
      iter = iter->left;
    } else if (key > iter->key) {
      iter = iter->right;
    } else {  // key == iter->key
      return iter;
    }
  }
}

/*========* rb_tree PRIVATE insert *========*/

bool rb_tree::insert(int key, rb_tree::node*& inserted_node) {
  if (root->is_nil()) {
    auto new_node = make_node(nullptr, key);
    root = new_node;
    return inserted_node = new_node, true;
  }
  auto iter = root;
  while (true) {
    if (key < iter->key) {
      if (iter->left->is_nil()) {
        iter->left = make_node(iter, key);
        return inserted_node = iter->left, true;
      } else {
        iter = iter->left;
      }
    } else if (key > iter->key) {
      if (iter->right->is_nil()) {
        iter->right = make_node(iter, key);
        return inserted_node = iter->right, true;
      } else {
        iter = iter->right;
      }
    } else {  // key == iter->key
      return false;
    }
  }
}

void rb_tree::insert_case1(rb_tree::node* n) {
  if (n->parent == nullptr) {
    n->color_ = color::BLACK;
  } else {
    insert_case2(n);
  }
}
void rb_tree::insert_case2(rb_tree::node* n) {
  // assert(n->parent != nullptr);
  if (n->parent->color_ == color::BLACK) {
    return; /* Tree is still valid */
  } else {
    insert_case3(n);
  }
}
void rb_tree::insert_case3(rb_tree::node* n) {
  // assert(n->parent != nullptr);
  // assert(n->parent=>color_ == color::RED);
  node* u = n->uncle();
  node* g;

  if ((u != nullptr) && (u->color_ == color::RED)) {
    n->parent->color_ = color::BLACK;
    u->color_ = color::BLACK;
    g = n->grandparent();
    g->color_ = color::RED;
    insert_case1(g);
  } else {
    insert_case4(n);
  }
}
void rb_tree::insert_case4(rb_tree::node* n) {
  // assert(n->parent != nullptr);
  // assert(n->parent=>color_ == color::RED);
  // assert((n->uncle() == nullptr) || (n->uncle()->color_ == color::BLACK));
  node* g = n->grandparent();

  if ((n == n->parent->right) && (n->parent == g->left)) {
    rotate_left(n->parent);
    n = n->left;

  } else if ((n == n->parent->left) && (n->parent == g->right)) {
    rotate_right(n->parent);
    n = n->right;
  }
  insert_case5(n);
}
void rb_tree::insert_case5(rb_tree::node* n) {
  node* g = n->grandparent();

  n->parent->color_ = color::BLACK;
  g->color_ = color::RED;
  if (n == n->parent->left) {
    rotate_right(g);
  } else {
    rotate_left(g);
  }
}
/*========* rb_tree PRIVATE remove *========*/
void rb_tree::deleteValue(rb_tree::node* n) {
  node* child = n->right->is_nil() ? n->left : n->right;

  swap_nodes(n, child);
  if (n->color_ == color::BLACK) {
    if (child->color_ == color::RED)
      child->color_ = color::BLACK;
    else
      delete_case1(child);
  }
  delete n;
}

void rb_tree::delete_case1(rb_tree::node* n) {
  // TODO
}
void rb_tree::delete_case2(rb_tree::node* n) {
  // TODO
}
void rb_tree::delete_case3(rb_tree::node* n) {
  // TODO
}
void rb_tree::delete_case4(rb_tree::node* n) {
  // TODO
}
void rb_tree::delete_case5(rb_tree::node* n) {
  // TODO
}
void rb_tree::delete_case6(rb_tree::node* n) {
  // TODO
}

/*========* rb_tree PRIVATE traversal *========*/

void rb_tree::prefix_traversal(std::function<void(const rb_tree::node&)> func,
                               const rb_tree::node* root) const {
  if (root->is_nil()) return;
  func(*root);
  prefix_traversal(func, root->left);
  prefix_traversal(func, root->right);
}
void rb_tree::infix_traversal(std::function<void(const rb_tree::node&)> func,
                              const rb_tree::node* root) const {
  if (root->is_nil()) return;
  infix_traversal(func, root->left);
  func(*root);
  infix_traversal(func, root->right);
}
void rb_tree::postfix_traversal(std::function<void(const rb_tree::node&)> func,
                                const rb_tree::node* root) const {
  if (root->is_nil()) return;
  postfix_traversal(func, root->left);
  postfix_traversal(func, root->right);
  func(*root);
}

/*========* various *========*/
std::ostream& operator<<(std::ostream& os, rb_tree& t) {
  os << "[ ";
  t.for_each([&os](const rb_tree::node& n) {
    os << n.key << "(" << (n.color_ == rb_tree::color::BLACK ? "B" : "R") << ")"
       << " ";
  });
  os << "]";
  return os;
}
static void debug_print(const rb_tree::node& n, int depth) {
  using namespace std;
  if (n.is_nil()) return;
  fill_n(ostream_iterator<char>{cout}, 2 * depth, ' ');
  const char* child_type =
      n.parent != nullptr ? n.parent->left == &n ? "left" : "right" : "root";
  cout << n.key << (n.color_ == rb_tree::color::BLACK ? "[B]" : "[R]") << " -["
       << child_type << "]> " << (n.parent == nullptr ? 0 : n.parent->key)
       << endl;
  debug_print(*n.left, depth + 1);
  debug_print(*n.right, depth + 1);
}

void debug_print(rb_tree& t) { debug_print(*t.get_root(), 0); }

void assert_is_search_tree(rb_tree& t) {
  t.for_each([](const auto& n) {
    assert(n.left->is_nil() || n.key > n.left->key);
    assert(n.right->is_nil() || n.key < n.right->key);
  });
}

void assert_parent_child_relationship_holds(rb_tree& t) {
  t.for_each([](const auto& n) {
    assert(n.left->parent == &n);
    assert(n.right->parent == &n);
  });
}

void assert_red_childs_are_black(rb_tree& t) {
  t.for_each([](const auto& n) {
    if (n.color_ == rb_tree::color::RED) {
      assert(n.left->color_ == rb_tree::color::BLACK);
      assert(n.right->color_ == rb_tree::color::BLACK);
    }
  });
}

static size_t assert_black_path_length_equal(rb_tree::node* n) {
  if (n->is_nil()) return 1;
  const auto left_count = assert_black_path_length_equal(n->left);
  const auto right_count = assert_black_path_length_equal(n->right);
  assert(left_count == right_count);
  return std::max(left_count, right_count) +
         (n->color_ == rb_tree::color::BLACK ? 1 : 0);
}

void assert_black_path_length_equal(rb_tree& t) {
  assert_black_path_length_equal(t.get_root());
}

template <typename Iter>
void fill_random(Iter first, Iter last, int min, int max) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution{min, max};

  std::generate(first, last, std::bind(distribution, generator));
}

void test1() {
  using namespace std;
  rb_tree t;
  int vals[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  // int vals[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
  // int vals[] = {5, 2, 7, 8, 4, 1, 4, 6, 9, 3};
  for (auto x : vals) {
    t.insert(x);
  }
  debug_print(t);
  // t.deleteValue(7);
  // debug_print(t);

  // for (auto x : vals) {
  //   t.deleteValue(x);
  //   debug_print(t);
  //   assert_parent_child_relationship_holds(t);
  //   assert_is_search_tree(t);
  //   assert_red_childs_are_black(t);
  //   assert_black_path_length_equal(t);
  // }
}

void test2() {
  using namespace std;
  constexpr auto min = 1;
  constexpr auto max = 100;
  constexpr auto count = 100;
  int vals[count];
  fill_random(begin(vals), end(vals), min, max);

  rb_tree t;
  for (auto x : vals) {
    t.insert(x);
    assert_parent_child_relationship_holds(t);
    assert_is_search_tree(t);
    assert_red_childs_are_black(t);
    assert_black_path_length_equal(t);
  }
  cout << "----------------" << endl;
  debug_print(t);
}

void test_insert_delete() {
  using namespace std;

  constexpr auto min = 1;
  constexpr auto max = 10000000;
  constexpr auto count = 10000000;

  rb_tree t;
  default_random_engine generator;
  uniform_int_distribution<int> distribution{min, max};
  auto next_int = bind(distribution, generator);

  vector<thread> threads;
  for (int i = 0; i < 1; ++i) {
    threads.push_back(thread{[&]() {
      for (auto i = 0; i < count; ++i) {
        t.insert(next_int());
      }
      // for (auto i = 0; i < count; ++i) {
      //   t.deleteValue(next_int());
      // }
    }});
  }

  join_all(threads);
}

void test_rtm_lock() {
  using namespace std;
  constexpr int count = 10000000;
  vector<int> values;
  values.resize(count, 0);
  LockElision lockElision;

  vector<thread> threads;
  for (int i = 0; i < 8; ++i) {
    threads.push_back(thread{[&]() {
      for (int i = 0; i < count; ++i) {
        RTMLock lock{lockElision};
        values[i]++;
      }
    }});
  }

  join_all(threads);
}

/*========* main *========*/
int main(int argc, char const* argv[]) {
  using namespace std;
  // test1();
  test_insert_delete();
  // test_rtm_lock();
  return 0;
}