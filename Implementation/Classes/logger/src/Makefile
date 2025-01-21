POSTGRESQL_DIR=../../con2db
CLOCK_DIR=../../Clock/src

CC=g++
CPPFLAGS=-std=c++20 -I. -I$(CLOCK_DIR) -I$(POSTGRESQL_DIR) -I/usr/include/postgresql -lpq

SRC=$(wildcard *.cpp)

DEPS=$(wildcard *.h)
DEPS_CLOCK := $(wildcard $(CLOCK_DIR)/*.h)
DEPS_POSTGRESQL := $(wildcard $(POSTGRESQL_DIR)/*.h)

OBJS=$(patsubst %.cpp, %.o, $(SRC))

all: $(OBJS)

clock:
	$(MAKE) -C ../../Clock/src

con2db:
	$(MAKE) -C ../../con2db

%.o: %.cpp $(DEPS) $(DEPS_CLOCK) $(DEPS_POSTGRESQL)
	$(CC) -c -o $@ $< $(CPPFLAGS)

.PHONY: clean

clean:
	rm *.o

