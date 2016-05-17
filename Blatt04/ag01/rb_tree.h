#ifndef C_BBTREE_H
#define C_BBTREE_H

#include <functional>

class rb_tree {
 public:
  enum class color : uint8_t { BLACK, RED };
  struct node {
    node* left;
    node* right;
    node* parent;
    int key;
    color color_;
    ~node();
    bool is_nil() const;
    node* grandparent() const;
    node* uncle() const;
  };

 public:
  rb_tree();
  ~rb_tree();
  bool insert(int key);
  bool search(node& n, int key);
  bool deleteValue(int key);
  void for_each(std::function<void(const node&)> func) const;
  void prefix_traversal(std::function<void(const node&)> func) const;
  void infix_traversal(std::function<void(const node&)> func) const;
  void postfix_traversal(std::function<void(const node&)> func) const;
  const node* get_root() const;

 private:
  node* root;

 private:
  static node* make_nil(node* parent);
  static node* make_node(node* parent, int key);

 private:
  bool insert(int key, node*& inserted_node);
  void insert_case1(node* n);
  void insert_case2(node* n);
  void insert_case3(node* n);
  void insert_case4(node* n);
  void insert_case5(node* n);
  void rotate_left(node* n);
  void rotate_right(node* n);

  void prefix_traversal(std::function<void(const node&)> func,
                        const node* root) const;
  void infix_traversal(std::function<void(const node&)> func,
                       const node* root) const;
  void postfix_traversal(std::function<void(const node&)> func,
                         const node* root) const;
};

#endif
