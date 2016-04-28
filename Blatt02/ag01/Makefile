OUT			= linkedlist
PARAMS      = -n 1 -t 1
CFLAGS 		= -Wall -g
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
	xfce4-terminal -x ~/bin/pause ./${OUT} ${PARAMS}

clean:
	rm -f ${OUT} *.o

.PHONY: all run clean

$(OUT): $(OBJ)
	g++ -o $(OUT) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

