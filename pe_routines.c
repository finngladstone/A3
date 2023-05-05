#include "pe_exchange.h"

trader * find_trader(int pid, struct trader * traders, int n) {
    for (int i = 0; i < n; i++) {
        if (&traders[i].pid == pid)
            return &traders[i];
    }

    return NULL;
}

char* id_command(char * src) {
    char* word;
    
    size_t len = strcspn(src, " ");
    word = (char*)malloc(len + 1);

    strncpy(word, src, len);
    word[len] = '\0';

    return word;
}