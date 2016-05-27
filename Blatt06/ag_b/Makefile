RUN=mpirun -n 2 ./jacobi_mpi
MPICXX=mpic++
CC=gcc
CXX=mpic++
RM=rm -f
CPPFLAGS=-g -std=c++11
LDFLAGS=-g -std=c++11
LDLIBS=-lboost_system -lboost_timer

SRCS=jacobi_mpi.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY=all run run_external clean

all: jacobi_mpi

run: all
	$(RUN)

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

jacobi_mpi.o: matrix.h

jacobi_mpi: $(OBJS)
	$(MPICXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)			

clean:
	$(RM) $(OBJS) jacobi_mpi

