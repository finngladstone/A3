#include "pe_exchange.h"

void MARKET_OPEN(trader * traders, int len) {
    for (int i = 0; i < len; i++) {
        write_data(traders[i].outgoing_fd, "MARKET OPEN;");
    }
}

void ORDERBOOK(trader * traders, int len) {
    /**
     * for product in products.txt
     *      - count number of buy orders 
     *      - count number of sell orders
     *      - collate orders with same price 
     * 
     */
}