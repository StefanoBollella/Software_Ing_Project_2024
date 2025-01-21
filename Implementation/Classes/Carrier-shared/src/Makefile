IDIR = .
ODIR = ../obj
BDIR = ../bin

CC=g++

CPPFLAGS=-std=c++20 -I.

DEPS=$(wildcard $(IDIR)/*.h)
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

all: $(OBJS)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)


#clean:
#	rm *.o

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o

# CFLAGS = -std=c++17 -Wall -I. -I$(IDIR) -I/usr/local/include -L/usr/local/lib -lyaml-cpp 

# all: $(BDIR)/carrier

#$(ODIR)/%.o: %.cpp $(DEPS)
#	$(cc) -c $< -o $@ $(CFLAGS)

#$(BDIR)/carrier: $(OBJ) $(DEPS)
#	$(cc) -o $@ $(OBJ) $(CFLAGS)
