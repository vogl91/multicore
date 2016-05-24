PARAMS=
RUN=mpirun  -np 1 -hostfile n3027_hostfile ./ctrl $(PARAMS) : -np 63 -hostfile n3027_hostfile ./wrk

MPICXX=mpic++
CC=gcc
CXX=mpic++
RM=rm -f
CPPFLAGS=-g -std=c++11 -O2 -Wall
LDFLAGS=-g -std=c++11 -lboost_system -lboost_timer 
LDLIBS=

WRK_SRCS=worker.cpp
CTRL_SRCS=controller.cpp
WRK_OBJS=$(subst .cpp,.o,$(WRK_SRCS))
CTRL_OBJS=$(subst .cpp,.o,$(CTRL_SRCS))

.PHONY=wrk ctrl run run_external all

all: wrk ctrl

run: all
	$(RUN)

run_external: all
	xfce4-terminal -x ~/bin/pause $(RUN)

wrk: matrix.h $(WRK_OBJS)
	$(MPICXX) -o $@ $(WRK_OBJS) $(LDFLAGS)

ctrl: matrix.h $(CTRL_OBJS)
	$(MPICXX) -o $@ $(CTRL_OBJS) $(LDFLAGS) 			

depend: .depend

clean:
	$(RM) $(WRK_OBJS) $(CTRL_OBJS) wrk ctrl

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
