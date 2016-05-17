#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>

class rb_tree {
 public:
  enum class color : uint8_t { BLACK, RED };
  struct node {
    node* left;
    node* right;
    node* parent;
    int key;
    color color_;
    bool is_nil() const;
  };

 public:
  rb_tree();
  bool insert(int key);
  bool search(node& n, int key);
  bool deleteValue(int key);
  void for_each(std::function<void(const node&)> func) const;
  void prefix_traversal(std::function<void(const node&)> func) const;
  void infix_traversal(std::function<void(const node&)> func) const;
  void postfix_traversal(std::function<void(const node&)> func) const;

 public:
  friend void debug_print(const rb_tree& t, bool print_color);

 private:
  node* root;

 private:
  static node NIL;
  static node* make_nil(node* parent);
  static node* make_node(node* parent, int key);

 private:
  bool insert(int key, node*& inserted_node);
  void prefix_traversal(std::function<void(const node&)> func,
                        const node* root) const;
  void infix_traversal(std::function<void(const node&)> func,
                       const node* root) const;
  void postfix_traversal(std::function<void(const node&)> func,
                         const node* root) const;
};

/*========* node *========*/
rb_tree::node rb_tree::NIL = {nullptr, nullptr, nullptr, 0,
                              rb_tree::color::BLACK};

bool rb_tree::node::is_nil() const {
  return left == nullptr && right == nullptr;
}
/*========* rb_tree INTERFACE *========*/

rb_tree::rb_tree() : root{make_nil(nullptr)} {}

bool rb_tree::insert(int key) {
  bool b;
  node* inserted_node = nullptr;
  b = insert(key, inserted_node);
  if (!b) return false;
  // TODO
  return true;
}
bool rb_tree::search(rb_tree::node& n, int key) {
  auto iter = root;
  while (true) {
    if (iter->is_nil()) {
      return false;
    } else if (key < iter->key) {
      iter = iter->left;
    } else if (key > iter->key) {
      iter = iter->right;
    } else {  // key == iter->key
      n = *iter;
      return true;
    }
  }
}
bool rb_tree::deleteValue(int key) {
  return false;  // TODO
}

void rb_tree::for_each(std::function<void(const rb_tree::node&)> func) const {
  infix_traversal(func, root);
}
void rb_tree::prefix_traversal(
    std::function<void(const rb_tree::node&)> func) const {
  infix_traversal(func, root);
}
void rb_tree::infix_traversal(
    std::function<void(const rb_tree::node&)> func) const {
  infix_traversal(func, root);
}
void rb_tree::postfix_traversal(
    std::function<void(const rb_tree::node&)> func) const {
  infix_traversal(func, root);
}
/*========* rb_tree PRIVATE *========*/
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

bool rb_tree::insert(int key, rb_tree::node*& inserted_node) {
  if (root->is_nil()) {
    auto new_node = make_node(&NIL, key);
    root = new_node;
    return inserted_node = new_node, true;
  }
  auto iter = root;
  while (true) {
    if (key < iter->key) {
      if (iter->left->is_nil()) {
        iter->left = make_node(iter, key);
        return true;
      } else {
        iter = iter->left;
      }
    } else if (key > iter->key) {
      if (iter->right->is_nil()) {
        iter->right = make_node(iter, key);
        return true;
      } else {
        iter = iter->right;
      }
    } else {  // key == iter->key
      return false;
    }
  }
}

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
std::ostream& operator<<(std::ostream& os, const rb_tree& t) {
  os << "[ ";
  t.for_each([&os](const rb_tree::node& n) {
    os << n.key << "(" << (n.color_ == rb_tree::color::BLACK ? "B" : "R") << ")"
       << " ";
  });
  os << "]";
  return os;
}
static void debug_print(const rb_tree::node& n, int depth, bool print_color) {
  using namespace std;
  if (n.is_nil()) return;
  fill_n(ostream_iterator<char>{cout}, 2 * depth, ' ');
  cout << n.key;
  if (print_color) cout << (n.color_ == rb_tree::color::BLACK ? "[B]" : "[R]");
  cout << endl;
  debug_print(*n.left, depth + 1, print_color);
  debug_print(*n.right, depth + 1, print_color);
}

void debug_print(const rb_tree& t, bool print_color) {
  debug_print(*t.root, 0, print_color);
}

/*========* main *========*/
int main(int argc, char const* argv[]) {
  using namespace std;
  rb_tree t;
  // int vals[] = {4, 2, 3, 1, 6, 7, 5};
  // int vals[] = {1, 2, 3, 4, 5, 6, 7};
  int vals[] = {7, 6, 5, 4, 3, 2, 1};
  for (auto x : vals) {
    t.insert(x);
    debug_print(t, true);
  }
  return 0;
}