#ifndef C_BBTREE_H
#define C_BBTREE_H

#include <functional>
#include <memory>

/**
 *
 * 1) Alle Blattknoten (NIL) sind schwarz. -> Correct by Design
 * 2) Ist ein Knoten rot, so sind beide Kinder schwarz. -> assert_children_black_if_node_red()
 * 3) Jeder Pfad von einem gegebenen Knoten zu seinen Blattknoten enthält die
 *     gleiche Anzahl schwarzer Knoten. -> TODO
 */
class Tree {
 public:
  enum class Color : uint8_t { RED, BLACK };
  struct node {
    std::shared_ptr<node> left;   // never nullptr if not NIL
    std::shared_ptr<node> right;  // never nullptr if not NIL
    int key;
    Color color;
    node(std::shared_ptr<node> left, std::shared_ptr<node> right, int key,
         Color color)
        : left{left}, right{right}, key{key}, color{color} {}
    node() : node{NIL, NIL, 0, Color::RED} {}
    node(const node& that) = default;
    node(node&& that) = default;
    node& operator=(const node& that) = default;
    node& operator=(node&& that) = default;
    bool is_nil() const { return this == NIL.get(); }
  };
  static const std::shared_ptr<node> NIL;

 public:
  Tree();
  bool insert(int key);
  bool search(int key, node& n);
  bool deleteValue(int key);
  void for_each(std::function<void(const Tree::node&)> func) const;
public:
	bool assert_children_black_if_node_red() const;
	bool assert_each_path_contains_same_number_of_black_nodes() const;
	bool test_assertions() const;
 private:
  std::shared_ptr<node> root;

 private:
  static std::shared_ptr<node> make_node(int key);
  void for_each(std::function<void(const Tree::node&)> func,
                std::shared_ptr<node> root) const;
  size_t assert_each_path_contains_same_number_of_black_nodes(const node& n, bool& b) const;
};

#endif
