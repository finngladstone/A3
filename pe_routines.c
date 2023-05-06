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