#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <thread>
#include <mutex>
#include "mcp.h"

template<bool threadSafe>
class Counter {
private:
    int count = 0;
    std::mutex m;
public:
    int get_count() const { return count; }
    
    Counter& operator++() {
        if(threadSafe)
            m.lock();
        count++;
        if(threadSafe)
            m.unlock();
        return *this;
    }
    void print() const {
        std::cout << get_count() << std::endl;
    }
};


template<bool threadSafe>
int run(int max_count, int thread_count) {
    using namespace std;
    vector<thread> threads;
    Counter<threadSafe> c;

    threads.reserve(thread_count);
    for(auto i=0; i<thread_count; ++i) {
        threads.emplace_back(thread{[&c,&max_count](){
            for(auto j=0;j<max_count;++j) {
                ++c;
            }
        }});
    }

    std::for_each(threads.begin(),threads.end(),[](thread& t){t.join();});

   c.print();
   return c.get_count();
}

int main(int argc, char **argv) {

    mcp_init(argc, argv);

    auto max_count = get_num_elements();
    auto thread_count = get_num_threads();
    
    auto thread_safe     = run<true>(max_count, thread_count);
    auto not_thread_safe = run<false>(max_count, thread_count);

    if(thread_safe != not_thread_safe) {
        std::cout << "race!!!" << std::endl;
    }

    return 0;
}
