#include "pe_exchange.h"

trader * find_trader(int pid, struct trader * traders, int n) {
    for (int i = 0; i < n; i++) {
        if (traders[i].pid == pid)
            return &traders[i];
    }

    return NULL;
}

int id_command(char * src, char * dest) {
    
    int len = strcspn(src, " ");

    if (len > 6) 
        return 0;

    strncpy(dest, src, len);
    dest[len] = '\0';

    return 1;
}

order * find_trader_order(trader * t, int order_id) {
    list_node* cursor = t->orders;
    
    while(cursor) {
        if (cursor->data.order->order_id == order_id) {
            return cursor->data.order;
        }
    }

    return NULL;
}

list_node * find_order_listnode(trader * t, int id) {
    list_node* cursor = t->orders;

    while(cursor) {
        if (cursor->data.order->order_id == id) {
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

void SEND_MARKET_OPEN(trader * traders, int n) {
    for (int i = 0; i < n; i++) {
        send_data(traders[i].outgoing_fd, "MARKET OPEN;");
        kill(traders[i].pid, SIGUSR1);
    }

}

void SEND_MARKET_UPDATE(trader * traders, int n, order o, trader * except) {
    char buffer[BUFFER_LEN] = {0};
    char * action;

    switch(o.type) {
        case BUY:
            action = "BUY";
            break;
        case SELL:
            action = "SELL";
            break;
        case CANCEL:
            action = "CANCEL";
            break;
    }

    snprintf(buffer, BUFFER_LEN-1, "MARKET %s %s %i %i;", 
        action, o.product->name, o.quantity, o.unit_cost);
    
    for (int i = 0; i < n; i++) {
        if (traders[i].online && &traders[i] != except) {
            send_data(traders[i].outgoing_fd, buffer);
            kill(traders[i].pid, SIGUSR1);
        }
    }
}

