CC=gcc
# CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
CFLAGS=-O3 -std=c11 -g
LDFLAGS=-lm
BINARIES=pe_exchange pe_trader
LL_OBJ=pe_ll.o pe_routines.o

all: $(BINARIES)

pe_exchange: pe_exchange.o $(LL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

pe_trader: pe_trader.o $(LL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARIES) *.o
