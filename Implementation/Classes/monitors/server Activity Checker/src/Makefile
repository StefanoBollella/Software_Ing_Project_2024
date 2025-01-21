#directory del server
IDIR = .
ODIR = ../obj
BDIR = ../bin


#Directory aggiuntive per i file sorgente e header
CON2REDIS_SRC_DIR = ../../../con2redis/src
CON2REDIS_OBJ_DIR = ../../../con2redis/obj

CLOCK_SRC_DIR = ../../../Clock/src
CLOCK_OBJ_DIR = ../../../Clock/obj

# è uguale al dir per OBJ
LOGGER_SRC_DIR = ../../../logger/src

POSTGRESQL_SRC_DIR = ../../../con2db

cc = g++
#Flag di compilazione
CFLAGS = -std=c++17 -Wall -Wextra -I$(IDIR) -I$(CON2REDIS_SRC_DIR) -I$(CLOCK_SRC_DIR) -I$(LOGGER_SRC_DIR) -I$(POSTGRESQL_SRC_DIR) -I/usr/local/include -I/usr/include/postgresql -L/usr/local/lib -lhiredis -lm -lpq

#Dipendenze
DEPS_CON2REDIS := $(wildcard $(CON2REDIS_SRC_DIR)/*.h) 
DEPS_CLOCK := $(wildcard $(CLOCK_SRC_DIR)/*.h)
DEPS_LOGGER := $(wildcard $(LOGGER_SRC_DIR)/*.h)
DEPS_POSTGRESQL := $(wildcard $(POSTGRESQL_SRC_DIR)/*.h)


#File sorgenti e file oggetto
SRC  = $(wildcard $(IDIR)/*.cpp)
OBJ  = $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

CON2REDIS_OBJ = $(wildcard $(CON2REDIS_OBJ_DIR)/*.o)
CLOCK_OBJ = $(wildcard $(CLOCK_OBJ_DIR)/*.o)
LOGGER_OBJ = $(wildcard $(LOGGER_SRC_DIR)/*.o)
POSTGRESQL_OBJ = $(wildcard $(POSTGRESQL_SRC_DIR)/*.o)


all: clock con2redis postgresql logger $(BDIR)/monitor

clock :
	$(MAKE) -C $(CLOCK_SRC_DIR)
con2redis :
	$(MAKE) -C $(CON2REDIS_SRC_DIR)
logger :
	$(MAKE) -C $(LOGGER_SRC_DIR)
postgresql :
	$(MAKE) -C $(POSTGRESQL_SRC_DIR)

$(ODIR)/%.o: $(IDIR)/%.cpp $(DEPS_CON2REDIS) $(DEPS_CLOCK) $(DEPS_LOGGER) $(DEPS_POSTGRESQL)
	$(cc) -c $< -o $@ $(CFLAGS)

$(BDIR)/monitor: $(OBJ) $(DEPS_CON2REDIS) $(DEPS_CLOCK) $(DEPS_LOGGER) $(DEPS_POSTGRESQL)
	$(cc) -o $@ $(OBJ) $(CON2REDIS_OBJ) $(CLOCK_OBJ) $(LOGGER_OBJ) $(POSTGRESQL_OBJ)  $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ core

