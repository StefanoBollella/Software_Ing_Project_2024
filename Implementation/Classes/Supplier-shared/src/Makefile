#Defines directories
IDIR = .
ODIR = ../obj
cc = g++

CFLAGS = -std=c++20 -Wall -Wextra -I$(IDIR) -I../../con2redis/src

SRC = $(wildcard $(IDIR)/*.cpp)
OBJ = $(patsubst $(IDIR)/%.cpp, $(ODIR)/%.o, $(SRC))
DEPS = $(wildcard $(IDIR)/*.h)

#Main target
all: deps $(OBJ)

deps:
	$(MAKE) -C ../../con2redis/src

#Rule for creating object files
$(ODIR)/%.o: $(IDIR)/%.cpp $(DEPS)
	$(cc) -c $< -o $@ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(IDIR)/*~ core

