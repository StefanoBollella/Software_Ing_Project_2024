ODIR = ../obj
BDIR = ../bin

CC = g++

CFLAGS = -std=c++20 -Wall -Wextra -Wsign-conversion -I. -I/usr/local/include -Lusr/local/lib

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, $(ODIR)/%.o, $(SRC))

all: $(ODIR)/$(OBJ)

$(ODIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o

