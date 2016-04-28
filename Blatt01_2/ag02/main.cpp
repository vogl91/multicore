#include <algorithm>
#include <condition_variable>
#include <iterator>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include "mcp.h"

template <typename Iter>
void join_all(Iter first, Iter last) {
  std::for_each(first, last, [](auto &t) {
    if (t.joinable()) t.join();
  });  // c++14 feature
}

class Barrier {
  // http://locklessinc.com/articles/barriers/
 private:
  static constexpr unsigned int BARRIER_FLAG = 1ul << 31;

 private:
  unsigned int count;
  unsigned int total;
  std::mutex m;
  std::condition_variable cv;

 public:
  Barrier(int number_of_threads)
      : count(number_of_threads), total(BARRIER_FLAG) {}
  /*return true if first to leave*/
  bool wait() {
    std::unique_lock<std::mutex> lock(m);
    while (total > BARRIER_FLAG) {  // for new threads while others are leaving
      cv.wait(lock);
    }
    if (total == BARRIER_FLAG) total = 0;  // first to enter
    total++;

    if (total == count) {
      total += BARRIER_FLAG - 1;
      cv.notify_all();
      return true;
    } else {
      while (total < BARRIER_FLAG) {
        cv.wait(lock);
      }
      total--;
      if (total == BARRIER_FLAG) {
        cv.notify_all();
      }
      return false;
    }
  }
};

template<typename Iter>
void print_all(Iter first, Iter last) {
  std::copy(first, last,
  std::ostream_iterator<double>(std::cout, " "));
  std::cout << std::endl;
  
}

template <typename Iter>
double calc_mean(Iter first, Iter last) {
  const auto N = static_cast<double>(last - first);
  const auto sum = static_cast<double>(std::accumulate(first, last, 0));
  return sum / N;
}

template <typename Iter>
double calc_variance(Iter first, Iter last, double expected_value) {
  const auto N = static_cast<double>(last - first);
  const auto expected_difference_sum =
      std::accumulate(first, last, 0, [expected_value](auto init, auto x) {
        return init += (x - expected_value) * (x - expected_value);
      });
  std::cout << expected_difference_sum << ", " << N << std::endl;
  return expected_difference_sum / N;
}

class Deviation {
 public:
  Deviation(const std::vector<double> &e);
  double get_deviation() const;

 private:
  using Iter = std::vector<double>::const_iterator;

 private:
  Barrier b;
  std::mutex m;

  std::vector<std::thread> threads;
  std::vector<double> local_results;
  size_t num_threads;
  size_t slice_size;
  double global_mean;
  double global_deviation;

 private:
  void do_work(const Iter first, const Iter last, size_t thread_index) {
    using namespace std;
    auto mean = calc_mean(first, last);
    local_results[thread_index] = mean;
    b.wait();
    if (thread_index == num_threads - 1) {
      cout << "local means: ";
      print_all(local_results.begin(), local_results.end());
      global_mean = std::accumulate(local_results.begin(), local_results.end(), 0) /
                    num_threads;
      cout << "global mean: " << global_mean << endl;
      // TODO: local mean of this (master) thread
    }
    b.wait();
    const auto variance = calc_variance(first, last, global_mean);
    local_results[thread_index] = variance;
    b.wait();

    if(thread_index == num_threads - 1) {
      cout << "local variance: ";
      print_all(local_results.begin(), local_results.end());
      cout << endl;
      global_deviation =
          sqrt(std::accumulate(local_results.begin(), local_results.end(), 0)/num_threads);
    }
  }
};

Deviation::Deviation(const std::vector<double> &e)
    : b(get_num_threads()), num_threads(get_num_threads()) {
  threads.resize(num_threads);
  local_results.resize(num_threads);

  slice_size = e.size() / num_threads;
  auto first = e.begin();
  auto last = first + slice_size;
  for (auto i = 0; i < num_threads - 1; ++i) {
    threads[i] = std::thread{&Deviation::do_work, this, first, last, i};
    first = last;
    last += slice_size;
  }
  do_work(first, e.end(), num_threads - 1);

  join_all(threads.begin(), threads.end());
}

double Deviation::get_deviation() const {
  return global_deviation;
}

// void func(Barrier *b) {
//   using namespace std;
//   cout << "0";
//   b->wait();
//   cout << ".";
//   b->wait();
//   cout << "1";
// }

// void test() {
//   using namespace std;
//   const int N = 10000;
//   Barrier b{N};

//   vector<thread> threads;
//   for (auto i = 0; i < N; ++i) {
//     threads.emplace_back(func, &b);
//   }

//   join_all(threads.begin(), threads.end());
// }
void test2() {
  using namespace std;
  vector<double> v{8,7,9,10,6};
  auto mean = calc_mean(v.begin(),v.end());
  cout << mean << " " << calc_variance(v.begin(),v.end(),mean) << endl;
}
int main(int argc, char **argv) {
  // test();
  test2();

  // initialize and parse arguments
  mcp_init(argc, argv);

  // initialize and shuffle elements
  std::vector<double> elements;
  for (int i = 0; i < num_elements; i++) {
    elements.push_back((double)(rand() % 1000));
  }

  // print all input elements (for debugging)
  std::copy(elements.begin(), elements.end(),
  std::ostream_iterator<double>(std::cout, " "));
  std::cout << std::endl;

  // start time measurement
  time_start();

  // get standard deviation of input elements
  Deviation d(elements);

  std::cout << d.get_deviation() << std::endl;

  // stop time measurement
  time_stop();

  // print time
  time_print();

  return 0;
}
