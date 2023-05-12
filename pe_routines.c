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

    if (buffer[n_read-1] == ';') {
        buffer[n_read-1] = '\0';
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

void SEND_FILL(trader * t, order * o, int quantity) {
    char buffer[BUFFER_LEN] = {0};
    snprintf(buffer, BUFFER_LEN-1, "FILL %i %i;", o->order_id, quantity);

    send_data(t->outgoing_fd, buffer);
    kill(t->pid, SIGUSR1);
}

position * find_position(trader * t, product * p) {
    list_node* cursor = t->positions;

    if (cursor == NULL) return NULL;

    while (cursor) {
        if (&cursor->data.position->item == &p)
            return cursor->data.position;  

        cursor = cursor->next; 
    }

    return NULL;
}

int number_of_live_traders(trader * traders, int n) {
    int c = 0;
    
    for (int i = 0; i < n; i++) {
        c+= traders[i].online;
    }

    return c;
}

int number_of_equal_orders(list_node * h, int c) {
    int i = 0;

    list_node * cursor = h;
    while (cursor != NULL) {
        if (cursor->data.order->unit_cost == c)
            i++;

        cursor = cursor->next;
    }

    return i;
}

void print_aggregate_orders(product * p) { 
    list_node * cursor;

    // SELL ORDERS FIRST
    cursor = list_get_tail(p->sell_orders);

    while (cursor != NULL) {
        order * o = cursor->data.order;
        int e = number_of_equal_orders(p->sell_orders, o->unit_cost);

        if (e == 1) {
            printf("%s\t\tSELL %i @ $%i (1 order)\n", LOG_PREFIX, o->quantity, o->unit_cost);
            cursor = cursor->prev;
        } 
        
        else {
            int sum_quantity = 0;
            for (int i = 0; i < e; i++) {
                sum_quantity += cursor->data.order->quantity;
                cursor = cursor->prev;
            }

            printf("%s\t\tSELL %i @ %i (%i orders)\n", LOG_PREFIX, sum_quantity, o->unit_cost, e);
        }
    }

    // NOW BUY ORDERS
    // THIS TIME TRAVERSE NORMALLY FROM HEAD - TAIL AS STORED IN DESC
    
    cursor = p->buy_orders;

    while (cursor != NULL) {
        order * o = cursor->data.order;
        int e = number_of_equal_orders(p->buy_orders, o->unit_cost);

        if (e == 1) {
            printf("%s\t\tBUY %i @ $%i (1 order)\n", LOG_PREFIX, o->quantity, o->unit_cost);
            cursor = cursor->next;
        } 
        
        else {
            int sum_quantity = 0;
            for (int i = 0; i < e; i++) {
                sum_quantity += cursor->data.order->quantity;
                cursor = cursor->next;
            }

            printf("%s\t\tBUY %i @ %i (%i orders)\n", LOG_PREFIX, sum_quantity, o->unit_cost, e);
        }
    }
    
}

void print_orderbook(list_node * product_ll) {
    printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);
    
    list_node * cursor = product_ll;
    while(cursor != NULL) {
        printf("%s\tProduct: %s; Buy levels: %i; Sell levels: %i\n", 
            LOG_PREFIX, cursor->data.product->name, list_get_len(cursor->data.product->buy_orders), 
                list_get_len(cursor->data.product->sell_orders));
        
        print_aggregate_orders(cursor->data.product);
        
        cursor = cursor->next;
    }
}

void print_positions(trader * traders, int n) {
    printf("%s\t--POSITIONS--\n", LOG_PREFIX);

    for (int i = 0; i < n; i++) { // iterate through traders
        trader * t = &traders[i];

        printf("%s\tTrader %i: ", LOG_PREFIX, t->id);
        list_node * cursor = t->positions;

        while (cursor != NULL) {
            position * p = cursor->data.position;

            printf("%s %i ($%i)", p->item->name, p->quantity, p->value);
            
            if (cursor->next != NULL)
                printf(", ");
            cursor = cursor->next;
        }

        printf("\n");
    }
}

void spx_report(list_node * product_ll, trader * traders, int n_traders) {
    print_orderbook(product_ll);
    print_positions(traders, n_traders);
}
 