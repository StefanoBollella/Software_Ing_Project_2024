#server directory
IDIR = .
ODIR = ../obj
BDIR = ../bin


#Directory aggiuntive per i file sorgente e header
CON2REDIS_SRC_DIR = ../../con2redis/src
CON2REDIS_OBJ_DIR = ../../con2redis/obj

SHARED_SRC_DIR = ../../Supplier-shared/src
SHARED_OBJ_DIR = ../../Supplier-shared/obj

CLOCK_SRC_DIR = ../../Clock/src
CLOCK_OBJ_DIR = ../../Clock/obj

CON2DB_DIR = ../../con2db

LOGGER_DIR = ../../logger/src



cc = g++
#Compilation flag
CFLAGS = -std=c++17 -Wall -Wextra -I$(IDIR) -I$(CON2REDIS_SRC_DIR) -I$(SHARED_SRC_DIR) -I$(CON2DB_DIR) -I$(LOGGER_DIR) -I$(CLOCK_SRC_DIR) -I/usr/local/include -I/usr/include/postgresql -L/usr/local/lib -lyaml-cpp -lhiredis -lm -lpq

#Dependencies
DEPS_CON2REDIS := $(wildcard $(CON2REDIS_SRC_DIR)/*.h) 
DEPS_SHARED := $(wildcard $(SHARED_SRC_DIR)/*.h)
DEPS_CLOCK := $(wildcard $(CLOCK_SRC_DIR)/*.h)
DEPS_CON2DB := $(wildcard $(CON2DB_DIR)/*.h)
DEPS_LOGGER := $(wildcard $(LOGGER_DIR)/*.h)

#Source and object files
SRC  = $(wildcard $(IDIR)/*.cpp)
OBJ  = $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

CON2REDIS_OBJ = $(wildcard $(CON2REDIS_OBJ_DIR)/*.o)
SHARED_OBJ = $(wildcard $(SHARED_OBJ_DIR)/*.o)
CLOCK_OBJ = $(wildcard $(CLOCK_OBJ_DIR)/*.o)
CON2DB_OBJ = $(wildcard $(CON2DB_DIR)/*.o)
LOGGER_OBJ = $(wildcard $(LOGGER_DIR)/*.o)

all: clock shared con2db logger $(BDIR)/supplier_server

clock :
	$(MAKE) -C $(CLOCK_SRC_DIR)
shared :
	$(MAKE) -C $(SHARED_SRC_DIR)
con2db :
	$(MAKE) -C $(CON2DB_DIR)
logger:
	$(MAKE) -C $(LOGGER_DIR)

$(ODIR)/%.o: $(IDIR)/%.cpp $(DEPS_CON2REDIS) $(DEPS_SHARED) $(DEPS_CON2DB) $(DEPS_LOGGER) $(DEPS_CLOCK) 
	$(cc) -c $< -o $@ $(CFLAGS)

$(BDIR)/supplier_server: $(OBJ) $(DEPS_CON2REDIS) $(DEPS_SHARED) $(DEPS_CON2DB) $(DEPS_LOGGER) $(DEPS_CLOCK)
	$(cc) -o $@ $(OBJ) $(CON2REDIS_OBJ) $(SHARED_OBJ) $(CON2DB_OBJ) $(LOGGER_OBJ) $(CLOCK_OBJ) $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ core

