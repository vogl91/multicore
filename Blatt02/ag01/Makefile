OUT			= linkedlist
# PARAMS      = -n 10000000 -t 8
PARAMS      = -n 40000 -t 8
CFLAGS 		= -Wall -g
CPPFLAGS 	= $(CFLAGS) -std=c++14 -fsanitize=leak
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

