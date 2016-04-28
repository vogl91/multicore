#include "mcp.h"
#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <stack>
#include <vector>

struct sudoku_field {
public:
  unsigned char data[81];

private:
  inline int index(int row, int col) const { return row * 9 + col; }
  inline void unset_bit_if_known(unsigned int &mask, int i) const {
    if (data[i] != UNKOWN) {
      mask &= (~(1u << (data[i] - 1)));
    }
  }

public:
  static const int UNKOWN = 0;

public:
  const unsigned char &operator()(int row, int col) const {
    return data[index(row, col)];
  }
  unsigned char &operator()(int row, int col) { return data[row * 9 + col]; }
  const unsigned char *begin() const { return data; }
  const unsigned char *end() const { return data + 81 * sizeof(data[0]); }
  unsigned int values_allowed(int row, int col) const {
    using namespace std;
    unsigned int mask = 0b111111111;
    // scan row
    for (auto i = index(row, 0); i < index(row, 9); i += 1) {
      unset_bit_if_known(mask, i);
    }
    // scan column
    for (auto i = index(0, col); i < index(9, col); i += 9) {
      unset_bit_if_known(mask, i);
    }
    // scan sub-grid
    auto x = (row / 3) * 3;
    auto y = (col / 3) * 3;
    for (auto xi = 0; xi < 3; ++xi) {
      for (auto yi = 0; yi < 3; ++yi) {
        auto i = index(x + xi, y + yi);
        unset_bit_if_known(mask, i);
      }
    }
    return mask;
  }
  bool is_complete() const {
    return std::none_of(begin(), end(),
                        [](const auto &cell) { return cell == UNKOWN; });
  }
};

std::ostream &operator<<(std::ostream &os, const sudoku_field &field) {
  for (auto row = 0; row < 9; ++row) {
    os << "|";
    for (auto col = 0; col < 9; ++col) {
      int val = field(row, col);
      if (val == sudoku_field::UNKOWN)
        os << " ";
      else
        os << val;
      if ((col + 1) % 3 == 0)
        os << "|";
    }
    os << std::endl;
    if ((row + 1) % 3 == 0)
      os << "|---+---+---|" << std::endl;
  }
  return os;
}

template <typename Func>
void for_each_set_bit(unsigned int mask, int max_bit, Func func) {
  for (auto i = 0; i < max_bit; ++i) {
    if (mask & 1 << i) {
      func(i);
    }
  }
}

template <typename Func>
bool for_each_empty_cell(const sudoku_field &field, Func func) {
  bool has_empty_cell = false;
  for (auto row = 0; row < 9; ++row) {
    for (auto col = 0; col < 9; ++col) {
      auto &cell = field(row, col);
      if (cell == sudoku_field::UNKOWN) {
        has_empty_cell = true;
        func(cell, row, col);
      }
    }
  }
  return has_empty_cell;
}

template <typename T> T pop(std::stack<T> &s) {
  auto x = std::move(s.top());
  s.pop();
  return std::move(x);
}

std::vector<sudoku_field> get_next_possible_fields(const sudoku_field &field) {
  std::vector<sudoku_field> fields;
  for_each_empty_cell(field, [&fields, &field](const unsigned char &cell,
                                               int row, int col) {
    auto allowed_values = field.values_allowed(row, col);
    for_each_set_bit(allowed_values, 9, [&fields, &field, &row, &col](int i) {
      fields.emplace_back(sudoku_field{field});
      fields.back()(row, col) = i + 1;
    });
  });
  return fields;
}

int main(int argc, char **argv) {
  using namespace std;
  // initialize and parse arguments
  mcp_init(argc, argv);

  // start time measurement
  time_start();

  // sudoku_field start_field = {{
  //     5, 3, 0, 0, 7, 0, 0, 0, 0, //
  //     6, 0, 0, 1, 9, 5, 0, 0, 0, //
  //     0, 9, 8, 0, 0, 0, 0, 6, 0, //
  //     8, 0, 0, 0, 6, 0, 0, 0, 3, //
  //     4, 0, 0, 8, 0, 3, 0, 0, 1, //
  //     7, 0, 0, 0, 2, 0, 0, 0, 6, //
  //     0, 6, 0, 0, 0, 0, 2, 8, 0, //
  //     0, 0, 0, 4, 1, 9, 0, 0, 5, //
  //     0, 0, 0, 0, 8, 0, 0, 7, 9  //
  // }};
  sudoku_field start_field = {{
      5, 3, 4, 6, 7, 8, 9, 1, 2, //
      6, 0, 2, 1, 9, 5, 3, 4, 8, //
      1, 9, 8, 3, 4, 2, 5, 6, 7, //
      8, 5, 9, 7, 0, 1, 4, 2, 3, //
      4, 2, 6, 8, 5, 3, 7, 9, 1, //
      7, 1, 3, 9, 2, 0, 8, 5, 6, //
      9, 6, 1, 5, 3, 7, 2, 0, 4, //
      2, 8, 7, 4, 1, 9, 6, 3, 5, //
      3, 4, 5, 2, 8, 6, 1, 7, 9  //
  }};
  /*sudoku_field start_field = {{
      5, 3, 4, 6, 7, 8, 9, 1, 2, //
      6, 7, 2, 1, 9, 5, 3, 4, 8, //
      1, 9, 8, 3, 4, 2, 5, 6, 7, //
      8, 5, 9, 7, 6, 1, 4, 2, 3, //
      4, 2, 6, 8, 5, 3, 7, 9, 1, //
      7, 1, 3, 9, 2, 4, 8, 5, 6, //
      9, 6, 1, 5, 3, 7, 2, 8, 4, //
      2, 8, 7, 4, 1, 9, 6, 3, 5, //
      3, 4, 5, 2, 8, 6, 1, 7, 9  //
  }};*/
  // cout << start_field << endl;

  stack<sudoku_field> field_stack;
  vector<sudoku_field> solutions;
  field_stack.push(start_field);

  while (!field_stack.empty()) {
    cout << field_stack.size() << " " << solutions.size() << endl;
    auto field = pop(field_stack);
    auto next  = get_next_possible_fields(field);
    for (auto &f : next) {
      if (f.is_complete())
        solutions.push_back(std::move(f));
      else
        field_stack.push(std::move(f));
    }
  }

  for (const auto &field : solutions) {
    cout << field << endl;
  }

  // stop time measurement
  time_stop();

  // print time
  time_print();

  return 0;
}
