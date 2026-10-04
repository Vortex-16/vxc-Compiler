CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Iinclude -g
TARGET=build/vxc
SRC=src/main.c src/lexer.c
all: $(TARGET)
$(TARGET): $(SRC) | build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
build:
	mkdir -p build
clean:
	rm -f $(TARGET)
