#include <iostream>
#include <vector>
#include <string>
#include "mcp.h"

using namespace std;

MCP_CLASS_::MCP_CLASS_() {
    threads = 0;
    elements = 0;
    timer_begin = {0,0};
    timer_end = {0,0};
    timer_sum = 0.0;
}

void MCP_CLASS_::init(int argc, char *argv[]) {
    vector<string> args(argv+1, argv+argc);
    for (unsigned int i=0; i<args.size(); i++) {
        if (args[i] == "-n") {
            if (i+1 < args.size()) {
                elements = stoi(args[i+1]);
                i++;
            }
        } else if (args[i] == "-t") {
            if (i+1 < args.size()) {
                threads = stoi(args[i+1]);
                i++;
            }
        }
    }

    if ((elements == 0) || (threads == 0)) {
        help(args[0]);
    }
}

void MCP_CLASS_::help(const string &p) {
    cout << "Usage: " << p << " [OPTION]..." << endl << endl;
    cout << "options:" << endl;
    cout << "  -n <NUM>    input size" << endl;
    cout << "  -t <NUM>    run <NUM> threads" << endl;
    exit(1);
}

int MCP_CLASS_::t() {
    return threads;
}

int MCP_CLASS_::n() {
    return elements;
}

void MCP_CLASS_::time_start() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer_begin);
}

void MCP_CLASS_::time_stop() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer_end);
    if(timer_end.tv_nsec < timer_begin.tv_nsec) {
        timer_end.tv_sec = timer_end.tv_sec - timer_begin.tv_sec - 1;
        timer_end.tv_nsec = 1000000000 + timer_end.tv_nsec - timer_begin.tv_nsec;
    } else {
        timer_end.tv_sec = timer_end.tv_sec - timer_begin.tv_sec;
        timer_end.tv_nsec = timer_end.tv_nsec - timer_begin.tv_nsec;
    }

    struct timespec sum;
    sum.tv_sec = timer_end.tv_sec;
    sum.tv_nsec = timer_end.tv_nsec;

    sum.tv_sec += sum.tv_nsec / 1000000000;
    sum.tv_nsec = sum.tv_nsec % 1000000000;
    timer_sum = sum.tv_sec + ((double)sum.tv_nsec / 1000000000.0);
}

void MCP_CLASS_::time_print() {

    printf("\n *** EXECUTION TIME: %.9lfs ***\n\n", timer_sum);
}

MCP_CLASS_::~MCP_CLASS_() {
    // time_print();
}


MCP_CLASS_ MCP;
