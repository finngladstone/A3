CC=gcc
CFLAGS= -D_POSIX_C_SOURCE=199309L -Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
LDFLAGS=-lm
BINARIES=pe_trader

all: $(BINARIES)

.PHONY: clean
clean:
	rm -f $(BINARIES)

