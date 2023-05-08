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

/** Comms framework */

void send_data(int fd, char * message) {
    if (write(fd, message, strlen(message)) == -1) {
        perror("send_data shit the bed");
    }
}

void receive_data(int fd, char * buffer) {
    ssize_t n_read;

    if ((n_read = read(fd, buffer, BUFFER_LEN)) == 1) {
        perror("receive_data failed");
        exit(2);
    }

    buffer[n_read] = '\0';
}

/** Comms commands  */

void SEND_STATUS(trader * t, int id, statuses s) {
    
    char * status;
    char buffer[BUFFER_LEN];

    switch(s) {
        case ACCEPTED:
            status = "ACCEPTED";
            break;
        case AMENDED:
            status = "AMENDED";
            break;
        case CANCELLED:
            status = "BREAK";
            break;
        case INVALID:
            send_data(t->outgoing_fd, "INVALID;");
            return;
    }

    snprintf(buffer, BUFFER_LEN, "%s %i;", status, id);
    send_data(t->outgoing_fd, buffer);

    kill(t->pid, SIGUSR1);
}

void MARKET_OPEN(trader * traders, int n) {
    for (int i = 0; i < n; i++) {
        send_data(traders[i].outgoing_fd, "MARKET OPEN;");
    }
}