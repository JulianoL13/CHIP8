CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Werror
LDFLAGS = `sdl2-config --cflags --libs`
SRC_DIR = src/cpu
BIN_DIR = bin

all: $(BIN_DIR)/chip8

$(BIN_DIR)/chip8: $(SRC_DIR)/chip8.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)/chip8
