#ifndef MCP_H
#define MCP_H

class MCP_CLASS_ {
    public:
        MCP_CLASS_();
        ~MCP_CLASS_();
        void init(int argc, char *argv[]);
        int t();
        int n();
        void time_start();
        void time_stop();
    private:
        int threads;
        int elements;
        struct timespec timer_begin;
        struct timespec timer_end;
        double timer_sum;
        void help(const std::string &p);
        void time_print();
};

extern MCP_CLASS_ MCP;

#endif

