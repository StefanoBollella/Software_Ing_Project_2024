REDIS_DIR = ../../con2redis/src 
SHARED_DIR = ../../Customer-shared/src
CLOCK_DIR = ../../Clock/src
ODIR = ../obj
BDIR = ../bin

CC = g++

# include -g and -ggdb flags for debugging with GDB
# CFLAGS = -std=c++20 -g -ggdb -Wall -Wextra -Werror -Wfatal-errors -fmax-errors=5 -Wsign-conversion

# Note: -O2 is an option for optimizing the code
# 	-lhiredis to include the redis library
# 	-lpq to use the library for postgresql
# 	-fsanitize=address for detecting memory leaks
# 	-fmax-errors=5
CFLAGS = -std=c++20 -g -Wall -Wextra -Wsign-conversion -I. -I$(REDIS_DIR) -I$(SHARED_DIR) -I$(CLOCK_DIR) -I/usr/local/include -L/usr/local/lib -lyaml-cpp -lhiredis

SRC  := $(wildcard *.cpp) 
DEPS_REDIS := $(wildcard $(REDIS_DIR)/*.h) 
DEPS_SHARED := $(wildcard $(SHARED_DIR)/*.h)
DEPS_CLOCK := $(wildcard $(CLOCK_DIR)/*.h)
OBJ  := $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

con2redis_OBJ := $(wildcard ../../con2redis/obj/*.o)
shared_OBJ := $(wildcard ../../Customer-shared/obj/*.o)
clock_OBJ := $(wildcard ../../Clock/obj/*.o)

all: $(BDIR)/customer

con2redis:
	$(MAKE) -C ../../con2redis/src

shared:
	$(MAKE) -C ../../Customer-shared/src

clock:
	$(MAKE) -C ../../Clock/src

$(ODIR)/%.o: %.cpp $(DEPS_REDIS) $(DEPS_SHARED) $(DEPS_CLOCK) 
	$(CC) -c $< -o $@ $(CFLAGS)

$(BDIR)/customer: $(OBJ) $(DEPS_REDIS) $(DEPS_SHARED) $(DEPS_CLOCK)
	$(CC) -o $@ $(OBJ) $(con2redis_OBJ) $(shared_OBJ) $(clock_OBJ) $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
