OUT			= deviation
PARAMS      = -n 1000 -t 4
CFLAGS 		= -Wall
CPPFLAGS 	= $(CFLAGS) -std=c++14
LIBS		= -lpthread
CC			= gcc
CPP			= g++

CSRC		= $(wildcard *.c)
CPPSRC		= $(wildcard *.cpp)
OBJ			= $(CSRC:.c=.o) $(CPPSRC:.cpp=.o)

all: $(OUT)

clean:
	rm -f *.o deviation deviation.exe

run: all
	./${OUT} ${PARAMS}

run_external: all
	xfce4-terminal -x ~/bin/pause "./${OUT} ${PARAMS}"


$(OUT): $(OBJ)
	g++ -o $(OUT) $(OBJ) $(LIBS)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
