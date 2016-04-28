#include <algorithm>
#include <numeric>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <vector>
#include "mcp.h"

template<typename Iter>
void join_all(Iter first, Iter last) {
  std::for_each(first,last,[](auto& t){if(t.joinable()) t.join();}); //c++14 feature
}

class Barrier {
private:
  int number_of_threads;
  int threads_left;
  std::mutex m;
  std::condition_variable cond_var;
private:
  bool condition() const { return threads_left == 0; }
public:
  Barrier(int number_of_threads) :
      number_of_threads(number_of_threads),
      threads_left(number_of_threads) {
  }
  void wait() {
    std::unique_lock<std::mutex> lock(m);
    threads_left--;
    while(!condition()) {
      cond_var.wait(lock);
    }
    cond_var.notify_all();
  }
};


template<typename Iter>
double calc_mean(Iter first, Iter last) {
  return static_cast<double>(std::accumulate(first, last, 0)) / static_cast<double>((last-first));
}

class Deviation {
public:
  Deviation(const std::vector<double> &e);
private:
  using Iter = std::vector<double>::const_iterator;
private:
  Barrier b;
private:
  void do_work(const Iter first, Iter last) {
    using namespace std;
    cout << "size" << (last-first) << endl;
    cout << calc_mean(first, last) << endl;
    b.wait();
  }
};

Deviation::Deviation(const std::vector<double> &e) : b(num_threads) {
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  int slice_size = e.size() / num_threads;
  auto first = e.begin();
  auto last = first + slice_size;
  for(auto i=0; i<num_threads-1; ++i) {
    threads.emplace_back(std::thread{&Deviation::do_work, this, first, last});
    first = last;
    last += slice_size;
  }
  
  do_work(last,e.end());
     
   
     
   join_all(threads.begin(),threads.end());
}

void func(Barrier* b) {
  using namespace std;
  cout << "0";
  b->wait();
  cout << ".";
}

void test() {
  using namespace std;
  const int N = 10000;
  Barrier b{N};

  vector<thread> threads;
  for(auto i=0; i<N; ++i) {
    threads.emplace_back(func,&b);
  }

  join_all(threads.begin(),threads.end());

}

int main(int argc, char **argv) {
  //test();


    // initialize and parse arguments
    mcp_init(argc, argv);
    
    // initialize and shuffle elements
    std::vector<double> elements;
    for (int i=0; i<num_elements; i++) {
        elements.push_back((double)(rand()%1000));
    }

    // print all input elements (for debugging)
    // std::copy(elements.begin(), elements.end(), std::ostream_iterator<double>(std::cout, " "));
    // std::cout << std::endl;
    
    // start time measurement
    time_start();

    // get standard deviation of input elements
    Deviation d(elements);

    // stop time measurement
    time_stop();

    // print time
    time_print();

    return 0;
}
