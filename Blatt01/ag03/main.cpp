#include <iostream>
#include "mcp.h"

// TODO

int main(int argc, char **argv) {

    // initialize and parse arguments
    mcp_init(argc, argv);
    
    // start time measurement
    time_start();

    // TODO solve sudokus...

    // stop time measurement
    time_stop();

    // print time
    time_print();

    return 0;
}
