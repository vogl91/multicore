OUT			= mergesort
# PARAMS      = -n 10000000 -t 1
PARAMS      = -n 1000000 -t 1
# PARAMS      = -n 100000 -t 1
# PARAMS      = -n 10000 -t 1
# PARAMS      = -n 1000 -t 1
# PARAMS      = -n 100 -t 1
# PARAMS      = -n 10 -t 1
CFLAGS 		= -Wall -O2
CPPFLAGS 	= $(CFLAGS) -std=c++14
LIBS		= -lpthread
CC			= gcc
CPP			= g++

CSRC		= $(wildcard *.c)
CPPSRC		= $(wildcard *.cpp)
OBJ			= $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

all: $(OUT)

run: all
	./${OUT} ${PARAMS}

run_external: all
	xfce4-terminal --maximize -x ~/bin/pause ./${OUT} ${PARAMS}

clean:
	rm -f ${OUT} *.o

.PHONY: all run run_external clean

$(OUT): $(OBJ)
	g++ -o $(OUT) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

