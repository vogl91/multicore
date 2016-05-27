RUN=mpirun -n 1 ./jacobi_seq
MPICXX=mpic++
CC=gcc
CXX=mpic++
RM=rm -f
CPPFLAGS=-g -std=c++11
LDFLAGS=-g -std=c++11
LDLIBS=-lboost_system  -lboost_timer

SRCS=jacobi_seq.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY=all run run_external clean

all: jacobi_seq

run: all
	$(RUN)

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

jacobi_seq.o: matrix.h

jacobi_seq: $(OBJS)
	$(MPICXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)			


clean:
	$(RM) $(OBJS) jacobi_seq


