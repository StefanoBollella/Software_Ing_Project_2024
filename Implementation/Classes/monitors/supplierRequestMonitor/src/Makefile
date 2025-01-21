#directory Monitor/src
IDIR = .
ODIR = ../obj
BDIR = ../bin


#Additional directories for source and header files
CLOCK_SRC_DIR = ../../../Clock/src
CLOCK_OBJ_DIR = ../../../Clock/obj
CON2DB_DIR = ../../../con2db
LOGGER_DIR = ../../../logger/src

cc = g++
#Compile flags
CFLAGS = -std=c++17 -Wall -Wextra -I$(IDIR) -I$(CON2DB_DIR) -I$(CLOCK_SRC_DIR) -I$(LOGGER_DIR) -I/usr/local/include -I/usr/include/postgresql -L/usr/local/lib -lm -lpq

#Addictions
DEPS_CLOCK := $(wildcard $(CLOCK_SRC_DIR)/*.h)
DEPS_CON2DB := $(wildcard $(CON2DB_DIR)/*.h)
DEPS_LOGGER := $(wildcard $(LOGGER_DIR)/*.h)

#Source files and object files
SRC  = $(wildcard $(IDIR)/*.cpp)
OBJ  = $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

CLOCK_OBJ = $(wildcard $(CLOCK_OBJ_DIR)/*.o)
CON2DB_OBJ = $(wildcard $(CON2DB_DIR)/*.o)
LOGGER_OBJ = $(wildcard $(LOGGER_DIR)/*.o)


all: clock con2db logger $(BDIR)/supplier_monitor

clock :
	$(MAKE) -C $(CLOCK_SRC_DIR)
con2db :
	$(MAKE) -C $(CON2DB_DIR)
logger:
	$(MAKE) -C $(LOGGER_DIR)

$(ODIR)/%.o: $(IDIR)/%.cpp $(DEPS_CON2DB) $(DEPS_CLOCK) $(DEPS_LOGGER)
	$(cc) -c $< -o $@ $(CFLAGS)

$(BDIR)/supplier_monitor: $(OBJ) $(DEPS_CON2DB) $(DEPS_CLOCK) $(DEPS_LOGGER)
	$(cc) -o $@ $(OBJ) $(CON2DB_OBJ) $(CLOCK_OBJ) $(LOGGER_OBJ) $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ core

