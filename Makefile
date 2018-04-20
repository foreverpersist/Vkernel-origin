# Makefile
# Author: Joy Allen

CC = gcc

INCLUDE = include

LIB = lib

STATIC_LIBS = $(wildcard $(LIB)/*.a)

DYNAMIC_LIBS = $(wildcard $(LIB)/*.so)

DEPENDENCIES = $(patsubst $(LIB)/lib%.so, -l%, $(DYNAMIC_LIBS))

SRC = src

OBJ = obj

BIN = bin

TARGET = $(patsubst $(SRC)/%.c, $(BIN)/%, $(wildcard $(SRC)/*.c))

all: $(OBJ) $(BIN) $(TARGET)

$(OBJ):
	mkdir $(OBJ)

$(BIN):
	mkdir $(BIN)

$(BIN)/%: $(OBJ)/%.o $(STATIC_LIBS)
	$(CC) -o  $@ $^ -L $(LIB) $(DEPENDENCIES)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -o $@ -c $^ -I $(INCLUDE)

clean:
	-rm $(TARGET)
	-rm $(OBJ)/*.o

.PYTHON: all clean
