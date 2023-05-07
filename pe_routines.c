#include "pe_exchange.h"

trader * find_trader(int pid, struct trader * traders, int n) {
    for (int i = 0; i < n; i++) {
        if (&traders[i].pid == &pid)
            return &traders[i];
    }

    return NULL;
}

void id_command(char * src, char * dest) {
    
    int len = strcspn(src, " ");

    if (len > 6) 
        dest = NULL;

    strncpy(dest, src, len);
    dest[len] = '\0';

    return;
}

order * find_trader_order(trader * t, int order_id) {
    list_node* cursor = t->orders;
    
    while(cursor) {
        if (cursor->data.order.order_id == order_id) {
            return &cursor->data.order;
        }
    }

    return NULL;
}

list_node * find_order_listnode(trader * t, int id) {
    list_node* cursor = t->orders;

    while(cursor) {
        if (cursor->data.order.order_id == id) {
            return cursor;
        }
    }

    return NULL;
}

// void SEND_ACCEPTED(trader t, int id) {
//     char buffer[BUFFER_LEN];
//     snprintf(buffer, 49, "ACCEPTED %i;", id);
//     write_data(t.outgoing_fd, buffer);
// }

// void SEND_MARKET_UPDATE(trader * traders, trader src) {

// }