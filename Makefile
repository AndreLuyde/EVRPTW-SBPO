# macros
CPP= g++
CFLAGS= -O3 -std=c++17 -fopenmp -Wall -pedantic
EXECUTABLE=program

# targets
all: solution.o main.o
	$(CPP) $(CFLAGS) -w -o $(EXECUTABLE) solution.o main.o -lm -lpthread

solution.o: solution/solution.cpp util/util.h solution/solution.h
	$(CPP) $(CFLAGS) -DIL_STD -c solution/solution.cpp

main.o: main.cpp  util/util.h solution/solution.h
	$(CPP) $(CFLAGS) -DIL_STD -c main.cpp

# remove
clean:
	rm -f ${EXECUTABLE} *.o

