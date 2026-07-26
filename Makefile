CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
LDFLAGS =
SRC     = src/main.c
BIN     = gameoflife

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BIN)

.PHONY: all clean
