NUM_PROCS=10
PARAMS=100000

RUN=mpirun -n $(NUM_PROCS) ./pi $(PARAMS)
MPICXX=mpic++
CC=gcc
CXX=mpic++
RM=rm -f
CPPFLAGS=-g -std=c++11 -O2
LDFLAGS=-g -std=c++11
LDLIBS=-lboost_system  -lboost_timer

SRCS=pi.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY=all run run_external clean

all: pi

run: all
	$(RUN)

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

pi: $(OBJS)
	$(MPICXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)			

clean:
	$(RM) $(OBJS) pi


