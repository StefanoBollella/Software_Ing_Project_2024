CLOCK_DIR = ../../../Clock/src
POSTGRESQL_DIR = ../../../con2db
LOGGER_DIR = ../../../logger/src
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
CFLAGS = -std=c++20 -O2 -Wall -Wextra -Wsign-conversion -I. -I$(CLOCK_DIR) -I$(POSTGRESQL_DIR) -I/usr/include/postgresql -I/usr/local/include -L/usr/local/lib -lpq

SRC  := $(wildcard *.cpp) 
DEPS := $(wildcard *.h)
DEPS_CLOCK := $(wildcard $(CLOCK_DIR)/*.h)
DEPS_POSTGRESQL := $(wildcard $(POSTGRESQL_DIR)/*.h)
DEPS_LOGGER := $(wildcard $(LOGGER_DIR)/*.h)
OBJ := $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

clock_OBJ := $(wildcard ../../../Clock/obj/*.o)
con2db_OBJ := $(wildcard ../../../con2db/*.o)
logger_OBJ := $(wildcard ../../../logger/src/*.o)

all: $(BDIR)/db-disk-usage-monitor

clock:
	$(MAKE) -C ../../../Clock/src

con2db:
	$(MAKE) -C ../../../con2db

$(ODIR)/%.o: $(SRC) $(DEPS) $(DEPS_CLOCK) $(DEPS_POSTGRESQL) $(DEPS_LOGGER)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BDIR)/db-disk-usage-monitor: $(OBJ) $(DEPS) $(DEPS_CLOCK) $(DEPS_POSTGRESQL) $(DEPS_LOGGER)
	$(CC) -o $@ $(OBJ) $(DEPS) $(clock_OBJ) $(con2db_OBJ) $(logger_OBJ) $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
