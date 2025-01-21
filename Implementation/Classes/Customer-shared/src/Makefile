CC=g++
CPPFLAGS=-std=c++20 -I.

OBJ_DIR=../obj

DEPS=$(wildcard *.h)
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

all: $(OBJS)

$(OBJ_DIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o
