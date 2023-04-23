#ifndef PE_TRADER_H
#define PE_TRADER_H

#include "pe_common.h"
#include <regex.h>

#define BUFFER_SIZE 128
#define PATH_LEN 25

#define BUY_ORDER "BUY %i %s %i %i;"

/* regex */
#define SELL_SYNTAX "^MARKET SELL ([A-Z]+) ([0-9]+) ([0-9]+);$"

// #define SELL_SYNTAX "^MARKET SELL ([a-zA-Z]+) (-?\d+) (-?\d+);$"
// "^place [A-S]([1-9]|1[0-9])$"

struct market_data {
    char name[BUFFER_SIZE];
    int quantity;
    int price;
};


#endif
