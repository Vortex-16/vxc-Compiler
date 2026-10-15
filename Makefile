CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Iinclude -g
TARGET=build/vxc
SRC=src/main.c src/lexer.c src/parser.c src/ast.c src/sema.c
.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

test: $(TARGET)
	python tests/test_runner.py

clean:
	rm -f $(TARGET)


