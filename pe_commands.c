#include "pe_exchange.h"

void MARKET_OPEN(trader * traders, int len) {
    for (int i = 0; i < len; i++) {
        write_data(traders[i].outgoing_fd, "MARKET OPEN;");
    }
}