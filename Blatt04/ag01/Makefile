OUT=rb_tree
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS= -g -O2 -mrtm -std=c++14
LDFLAGS= -g -O2
LDLIBS= -lpthread

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

.PHONY: all depend clean dist-clean record report

all: rb_tree

run: all
	./${OUT} ${PARAMS}

run_external: all
	xfce4-terminal -x ~/bin/pause ./${OUT} ${PARAMS}

rb_tree: $(OBJS)
	$(CXX)  -o $@ $(OBJS) $(LDFLAGS)  $(LDLIBS)

profile: rb_tree
	perf stat -e tx-start,tx-commit,tx-abort,tx-conflict,tx-capacity,cycles-t,cycles-ct `pwd`/$^ 2>&1 

record: rb_tree
	perf record -e tx-start,tx-commit,tx-abort,tx-conflict,tx-capacity,cycles-t,cycles-ct `pwd`/$^

report: record 
	perf report

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) rb_tree

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
