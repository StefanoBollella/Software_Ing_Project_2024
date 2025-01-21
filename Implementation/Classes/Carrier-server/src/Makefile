REDIS_DIR = ../../con2redis/src 
SHARED_DIR = ../../Carrier-shared/src
CLOCK_DIR = ../../Clock/src
POSTGRESQL_DIR = ../../con2db
LOGGER_DIR = ../../src

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
##CFLAGS = -std=c++17 -g -Wall -Wextra -Wsign-conversion -I. -I$(REDIS_DIR) -I$(SHARED_DIR) -I$(CLOCK_DIR) -I/usr/local/include -L/usr/local/lib -lyaml-cpp -lhiredis
CFLAGS = -std=c++17 -O2 -Wall -Wextra -Wsign-conversion -I. -I$(REDIS_DIR) -I$(CLOCK_DIR) -I$(SHARED_DIR) -I$(POSTGRESQL_DIR) -I$(LOGGER_DIR) -I/usr/include/postgresql -I/usr/local/include -L/usr/local/lib -lhiredis -lpq

SRC  := $(wildcard *.cpp) 
DEPS_REDIS := $(wildcard $(REDIS_DIR)/*.h) 
DEPS_SHARED := $(wildcard $(SHARED_DIR)/*.h)
DEPS_CLOCK := $(wildcard $(CLOCK_DIR)/*.h)
DEPS_POSTGRESQL := $(wildcard $(POSTGRESQL_DIR)/*.h)
DEPS_LOGGER := $(wildcard $(LOGGER_DIR)/*.h)
OBJ  := $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

con2redis_OBJ := $(wildcard ../../con2redis/obj/*.o)
shared_OBJ := $(wildcard ../../Carrier-shared/obj/*.o)
clock_OBJ := $(wildcard ../../Clock/obj/*.o)
con2db_OBJ := $(wildcard ../../con2db/*.o)
logger_OBJ := $(wildcard ../../logger/src/*.o)

all: $(BDIR)/server

con2redis:
	$(MAKE) -C ../../con2redis/src

shared:
	$(MAKE) -C ../../Carrier-shared/src

clock:
	$(MAKE) -C ../../Clock/src

con2db:
	$(MAKE) -C ../../con2db

logger:
	$(MAKE) -C ../../src

$(ODIR)/%.o: %.cpp $(DEPS_REDIS) $(DEPS_SHARED) $(DEPS_CLOCK) $(DEPS_POSTGRESQL) $(DEPS_LOGGER)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BDIR)/server: $(OBJ) $(DEPS_REDIS) $(DEPS_SHARED) $(DEPS_CLOCK) $(DEPS_POSTGRESQL) $(DEPS_LOGGER)
	$(CC) -o $@ $(OBJ) $(con2redis_OBJ) $(shared_OBJ) $(clock_OBJ) $(con2db_OBJ) $(logger_OBJ)  $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o


