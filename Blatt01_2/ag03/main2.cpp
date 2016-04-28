#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>
#include <tuple>
#include <vector>
using namespace std;

class sudoku_field {
public:
  using cell_t                    = unsigned char;
  static constexpr size_t SIZE    = 81;
  static constexpr cell_t UNKNOWN = 0;

private:
  std::unique_ptr<cell_t[]> data;

private:
  inline int index(int row, int col) const { return row * 9 + col; }
  inline void unset_bit_if_known(unsigned int &mask, int i) const {
    if (data[i] != UNKNOWN) {
      mask &= (~(1u << (data[i] - 1)));
    }
  }

public:
  auto begin() { return data.get(); }
  auto begin() const { return data.get(); }
  auto cbegin() const { return data.get(); }
  auto end() { return data.get() + SIZE; }
  auto end() const { return data.get() + SIZE; }
  auto cend() const { return data.get() + SIZE; }

  sudoku_field() : data{new cell_t[SIZE]} {}
  sudoku_field(cell_t values[]) : data{new cell_t[SIZE]} {
    memcpy(data.get(), values, SIZE * sizeof(cell_t));
  }
  sudoku_field(const sudoku_field &that) : sudoku_field(that.data.get()) {}
  sudoku_field(sudoku_field &&that) : data(std::move(that.data)) {}
  sudoku_field(std::initializer_list<cell_t> list) : sudoku_field() {
    // static_assert(SIZE == list.size(),"list must contain sudoku_field::SIZE
    // elements");
    std::copy(list.begin(), list.end(), data.get());
  }
  sudoku_field &operator=(const sudoku_field &that) {
    std::copy(that.begin(), that.end(), begin());
    return *this;
  }
  sudoku_field &operator=(sudoku_field &&that) {
    data = std::move(that.data);
    return *this;
  }

  auto get(int row, int col) -> cell_t & { return data[index(row, col)]; }
  auto get(int row, int col) const -> const cell_t & {
    return data[index(row, col)];
  }
  auto operator()(int row, int col) -> cell_t & { return get(row, col); }
  auto operator()(int row, int col) const -> const cell_t & {
    return get(row, col);
  }

  unsigned int allowed_values(int row, int col) const {
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
    // scan zone
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
    return std::none_of(data.get(), data.get() + SIZE,
                        [](const auto &cell) { return cell == UNKNOWN; });
  }
};

template <typename T> class thread_safe_stack {
private:
  std::stack<T> stack;
  std::mutex m;

public:
  void push(const T &t) {
    std::lock_guard<std::mutex> lock{m};
    stack.push(t);
  }
  void push(T &&t) {
    std::lock_guard<std::mutex> lock{m};
    stack.push(t);
  }
  bool pop_if_not_empty(T &t) {
    std::lock_guard<std::mutex> lock{m};
    if (stack.empty())
      return true;
    t = std::move(stack.top());
    stack.pop();
    return false;
  }
};

template <typename T> class thread_safe_vector {
private:
  std::vector<T> vector;
  std::mutex m;

public:
  void push_back(const T &t) {
    std::lock_guard<std::mutex> lock{m};
    vector.push_back(t);
  }
  void push(T &&t) {
    std::lock_guard<std::mutex> lock{m};
    vector.push_back(t);
  }
  auto begin() { return vector.begin(); }
  auto begin() const { return vector.begin(); }
  auto cbegin() const { return vector.cbegin(); }
  auto end() { return vector.end(); }
  auto end() const { return vector.end(); }
  auto cend() const { return vector.cend(); }
};

std::pair<int, int> find_first_unknown_cell_coords(const sudoku_field &field) {
  for (int row = 0; row < 9; ++row) {
    for (int col = 0; col < 9; ++col) {
      const auto &cell = field(row, col);
      if (cell == sudoku_field::UNKNOWN)
        return {row, col};
    }
  }
  return {-1, -1};
}

template <typename Func>
void for_each_set_bit(unsigned int mask, int bit_count, Func func) {
  for (auto i = 0; i < bit_count; ++i) {
    if (mask & 1 << i) {
      func(i);
    }
  }
}

std::vector<sudoku_field> get_next_fields(const sudoku_field &field) {
  std::vector<sudoku_field> fields;
  int row, col;
  std::tie(row, col) = find_first_unknown_cell_coords(field);
  auto mask = field.allowed_values(row, col);
  for_each_set_bit(mask, 9, [&fields, &field, &row, &col](int bit_i) {
    fields.emplace_back(field);
    fields.back()(row, col) = bit_i + 1;
  });
  return fields;
}

bool step(thread_safe_stack<sudoku_field> &remaining_fields,
          thread_safe_vector<sudoku_field> &complete_fields) {
  sudoku_field field;
  bool empty = remaining_fields.pop_if_not_empty(field);
  if (empty) {
    return false;
  }
  auto next_fields = get_next_fields(field);
  for (auto &f : next_fields) {
    if (f.is_complete())
      complete_fields.push_back(std::move(f));
    else
      remaining_fields.push(std::move(f));
  }
  return true;
}

std::ostream &operator<<(std::ostream &os, const sudoku_field &field) {
  for (auto row = 0; row < 9; ++row) {
    os << "|";
    for (auto col = 0; col < 9; ++col) {
      int val = field(row, col);
      if (val == sudoku_field::UNKNOWN)
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

template <typename Iter> void join_all(Iter first, Iter last) {
  std::for_each(first, last, [](auto &t) {
    if (t.joinable())
      t.join();
  });
}

int main(int argc, char const *argv[]) {
  sudoku_field start_field = {
      5, 3, 0, 0, 7, 0, 0, 0, 0, //
      6, 0, 0, 1, 9, 5, 0, 0, 0, //
      0, 9, 8, 0, 0, 0, 0, 6, 0, //
      8, 0, 0, 0, 6, 0, 0, 0, 3, //
      4, 0, 0, 8, 0, 3, 0, 0, 1, //
      7, 0, 0, 0, 2, 0, 0, 0, 6, //
      0, 6, 0, 0, 0, 0, 2, 8, 0, //
      0, 0, 0, 4, 1, 9, 0, 0, 5, //
      0, 0, 0, 0, 8, 0, 0, 7, 9  //
  };
  int number_of_threads = 1;

  cout << start_field << endl;
  thread_safe_vector<sudoku_field> complete_fields;
  thread_safe_stack<sudoku_field> remaining_fields;
  remaining_fields.push(start_field);

  std::vector<std::thread> threads;
  for (int i = 0; i < number_of_threads; ++i) {
    threads.push_back(thread{[&]() {
      while (step(remaining_fields, complete_fields))
        ;
    }});
  }
  join_all(threads.begin(), threads.end());

  for (const auto &f : complete_fields) {
    cout << f << endl;
  }


  return 0;
}
