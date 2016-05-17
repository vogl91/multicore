#include <functional>
#include <iostream>

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

 private:
  node* root;

 private:
  static node NIL;
  static node* make_nil(node* parent);
  static node* make_node(node* parent, int key);

 private:
  bool insert(int key, node*& inserted_node);
  void for_each(std::function<void(const node&)> func, node* root) const;
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

  return true;  // TODO
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
  for_each(func, root);
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

void rb_tree::for_each(std::function<void(const rb_tree::node&)> func,
                       rb_tree::node* root) const {
  if (root->is_nil()) return;
  for_each(func, root->left);
  func(*root);
  for_each(func, root->right);
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

/*========* main *========*/
int main(int argc, char const* argv[]) {
  using namespace std;
  rb_tree t;

  t.insert(1);
  t.insert(3);
  t.insert(5);
  t.insert(2);
  t.insert(4);
  cout << t << endl;
  return 0;
}