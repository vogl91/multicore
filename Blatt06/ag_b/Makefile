# RUN=mpirun  -np 16 -hostfile n3027_hostfile ./jacobi_mpi
RUN=mpirun  -np 16 -hostfile n3027_hostfile ./jacobi_mpi 100 matrix.txt
# RUN=mpirun -n 4 ./jacobi_mpi
MPICXX=mpic++
CC=gcc
CXX=mpic++
RM=rm -f
CPPFLAGS=-g -std=c++14
LDFLAGS=-g -std=c++14
LDLIBS=-lboost_system -lboost_timer

SRCS=jacobi_mpi.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY=all run run_external clean

all: jacobi_mpi create_matrix

run: all
	$(RUN)

create_matrix: create_matrix.o
	g++ $(CPPFLAGS) create_matrix.o -o create_matrix

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

jacobi_mpi.o: matrix.h
create_matrix.o: matrix.h

jacobi_mpi: $(OBJS)
	$(MPICXX) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)			

clean:
	$(RM) $(OBJS) jacobi_mpi


