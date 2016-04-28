OUT			= sudoku
CFLAGS 		= -Wall
CPPFLAGS 	= $(CFLAGS) -std=c++11
LIBS		= -lpthread
CC			= gcc
CPP			= g++

CSRC		= $(wildcard *.c)
CPPSRC		= $(wildcard *.cpp)
OBJ			= $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

all: $(OUT)

$(OUT): $(OBJ)
	g++ -o $(OUT) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
