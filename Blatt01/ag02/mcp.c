#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int num_elements;
int num_threads;

static struct timespec timer_begin;
static struct timespec timer_end;
static struct timespec timer_sum = { 0, 0 };

void help(char *p) {
    printf("Usage: %s [OPTION]...\n\n", p);
    printf("options:\n");
    printf("  -n <NUM>    input size\n");
    printf("  -t <NUM>    run <NUM> threads\n");
    exit(1);
}

void mcp_init(int argc, char **argv) {
    int i;

    num_elements = 0;
    num_threads = 0;

    for (i=1; i<argc; i++) {
        if (strncmp(argv[i], "-n", 3) == 0) {
            if (i+1 < argc) {
                num_elements = atoi(argv[i+1]);
                i++;
            }
        } else if (strncmp(argv[i], "-t", 3) == 0) {
            if (i+1 < argc) {
                num_threads = atoi(argv[i+1]);
                i++;
            }
        } else {
            help(argv[0]);
        }
    }

    if ((num_elements == 0) || (num_threads == 0)) {
        help(argv[0]);
    }

}

void time_start() {
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &timer_begin) != 0) {
        fprintf(stderr, "cannot get the time\n");
    }
}

void time_stop() {
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &timer_end) != 0) {
        fprintf(stderr, "cannot get the time\n");
        return;
    }
    if(timer_end.tv_nsec < timer_begin.tv_nsec) {
        timer_end.tv_sec = timer_end.tv_sec - timer_begin.tv_sec - 1;
        timer_end.tv_nsec = 1000000000 + timer_end.tv_nsec - timer_begin.tv_nsec;
    } else {
        timer_end.tv_sec = timer_end.tv_sec - timer_begin.tv_sec;
        timer_end.tv_nsec = timer_end.tv_nsec - timer_begin.tv_nsec;
    }
    timer_sum.tv_sec += timer_end.tv_sec;
    timer_sum.tv_nsec += timer_end.tv_nsec;
}

double time_get() {
    double s;
    timer_sum.tv_sec += timer_sum.tv_nsec / 1000000000;
    timer_sum.tv_nsec = timer_sum.tv_nsec % 1000000000;
    s = timer_sum.tv_sec + ((double)timer_sum.tv_nsec / 1000000000.0);
    return s;
}

void time_print() {
    double s;

    s = time_get();
    printf("\n *** EXECUTION TIME: %.9lfs ***\n\n", s);
}

int get_num_threads() {
    return num_threads;
}

int get_num_elements() {
    return num_elements;
}

