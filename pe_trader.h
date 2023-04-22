#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"

#define BUFFER_SIZE 128
#define PATH_LEN 25

/* regex */
#define SELL_SYNTAX "^MARKET SELL ([a-zA-Z]+) (\d+) (\d+);$"
// #define SELL_SYNTAX "^MARKET SELL ([a-zA-Z]+) (-?\d+) (-?\d+);$"

#endif
