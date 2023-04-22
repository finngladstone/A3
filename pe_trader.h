#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"
#include <regex.h>

#define BUFFER_SIZE 128
#define PATH_LEN 25

#define BUY_ORDER "BUY %i %s %i %i;"

/* regex */
#define SELL_SYNTAX "^MARKET SELL ([a-zA-Z]+) (\d+) (\d+);$"
// #define SELL_SYNTAX "^MARKET SELL ([a-zA-Z]+) (-?\d+) (-?\d+);$"

struct market_data {
    char name[BUFFER_SIZE];
    int quantity;
    int price;
};


#endif
