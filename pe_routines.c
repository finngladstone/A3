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

void SEND_ACCEPTED(trader t, int id) {
    char buffer[BUFFER_LEN];
    snprintf(buffer, 49, "ACCEPTED %i;", id);
    write_data(t.outgoing_fd, buffer);
}

void SEND_MARKET_UPDATE(trader * traders, trader src) {

}