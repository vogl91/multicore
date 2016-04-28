OUT			= monitor
CFLAGS 		= -Wall
CPPFLAGS 	= $(CFLAGS) -std=c++11
LIBS		= -lpthread
CC			= gcc
CPP			= g++

CSRC		= $(wildcard *.c)
CPPSRC		= $(wildcard *.cpp)
OBJ			= $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

all: $(OUT)

run: all
	./monitor -n 1000000 -t 10

clean:
	rm -f *.o monitor.exe monitor

.PHONY: run clean

$(OUT): $(OBJ)
	g++ -o $(OUT) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

