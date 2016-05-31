# RUN=mpirun  -np 16 -hostfile n3027_hostfile ./jacobi_thrds 100
RUN=mpirun -np 16 -hostfile n3027_hostfile ./jacobi_thrds 100 matrix.txt
# RUN=./create_matrix 100 > matrix.txt
# RUN=mpirun -n 4 ./jacobi_thrds
MPICXX=mpic++
CC=gcc
CXX=mpic++ -fopenmp
RM=rm -f
CPPFLAGS=-g -std=c++14 -O2
LDFLAGS=-g -std=c++14 -O2
LDLIBS=-lboost_system -lboost_timer -lgomp

SRCS=jacobi_thrds.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY=all run run_external clean

all: jacobi_thrds create_matrix

run: all
	$(RUN)

create_matrix: create_matrix.o
	g++ $(CPPFLAGS) create_matrix.o -o create_matrix

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

jacobi_thrds.o: matrix.h
create_matrix.o: matrix.h

jacobi_thrds: $(OBJS)
	$(MPICXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)			

clean:
	$(RM) $(OBJS) jacobi_thrds


