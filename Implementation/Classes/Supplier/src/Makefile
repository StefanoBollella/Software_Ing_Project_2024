#Directory of suppliers
IDIR = .
ODIR = ../obj
BDIR = ../bin
cc = g++

#Additional directories for source and header files
CON2REDIS_SRC_DIR = ../../con2redis/src
CON2REDIS_OBJ_DIR = ../../con2redis/obj

SHARED_SRC_DIR = ../../Supplier-shared/src
SHARED_OBJ_DIR = ../../Supplier-shared/obj

CLOCK_SRC_DIR = ../../Clock/src
CLOCK_OBJ_DIR = ../../Clock/obj

#Compilation flag
CFLAGS = -std=c++17 -Wall -Wextra -I. -I$(IDIR) -I$(CON2REDIS_SRC_DIR) -I$(SHARED_SRC_DIR) -I$(CLOCK_SRC_DIR) -I/usr/local/include -L/usr/local/lib -lyaml-cpp -lhiredis -lm


#Dependencies
DEPS_CON2REDIS := $(wildcard $(CON2REDIS_SRC_DIR)/*.h) 
DEPS_SHARED := $(wildcard $(SHARED_SRC_DIR)/*.h)
DEPS_CLOCK := $(wildcard $(CLOCK_SRC_DIR)/*.h)


#Source and object files
SRC  = $(wildcard $(IDIR)/*.cpp)
OBJ  = $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

CON2REDIS_OBJ = $(wildcard $(CON2REDIS_OBJ_DIR)/*.o)
SHARED_OBJ = $(wildcard $(SHARED_OBJ_DIR)/*.o)
CLOCK_OBJ = $(wildcard $(CLOCK_OBJ_DIR)/*.o)

all: clock shared $(BDIR)/supplier

clock :
	$(MAKE) -C $(CLOCK_SRC_DIR)
shared :
	$(MAKE) -C $(SHARED_SRC_DIR)

$(ODIR)/%.o: $(IDIR)/%.cpp $(DEPS_CON2REDIS) $(DEPS_SHARED) $(DEPS_CLOCK)
	$(cc) -c $< -o $@ $(CFLAGS)

$(BDIR)/supplier: $(OBJ) $(DEPS_CON2REDIS) $(DEPS_SHARED) $(DEPS_CLOCK)
	$(cc) -o $@ $(OBJ) $(CON2REDIS_OBJ) $(SHARED_OBJ) $(CLOCK_OBJ) $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ core

