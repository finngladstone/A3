#include "pe_exchange.h"

/**
 * This file contains a myriad of helper functions that are
 * not associated with the linked list framework
 * 
 */

/** Data structures initialisation */

list_node* init_products(const char * filename) {

    list_node* head = NULL;  
    FILE * myfile;
    if ((myfile = fopen(filename, "r")) == NULL) {
        perror("Error opening product file");
        exit(1);
    }

    int n;
    if (fscanf(myfile, "%d\n", &n) != 1) {
        perror("Failed to read number of products");
        exit(2);
    }

    char buffer[BUFFER_LEN] = {0};

    int i;
    for (i = 0; i < n; i++) {
        fgets(buffer, BUFFER_LEN, myfile);
        buffer[strlen(buffer)-1] = '\0';

        product * p = calloc(1, sizeof(product));
        strcpy(p->name, buffer);
        p->buy_orders = NULL;
        p->sell_orders = NULL;

        list_add(&head, p, PRODUCT);
    }

    printf("%s Trading %d products: ", LOG_PREFIX, i);

    list_node* c = head;
    while(c){
        if (c->next == NULL)
            printf("%s\n", c->data.product->name);
        else   
            printf("%s ", c->data.product->name);
        
        c = c->next;
    }

    fclose(myfile);
    return head;
}

struct trader* get_traders(int argc, char const *argv[], list_node * product_ll) {
    struct trader * traders = malloc((argc - 2) * sizeof(struct trader));

    for (int i = 2; i < argc; i++) {
        traders[i-2].id = i-2;
        strcpy(traders[i-2].path, argv[i]);
        
        traders[i-2].next_order_id = 0;
        traders[i-2].orders = NULL;
		traders[i-2].positions = NULL;

        traders[i-2].online = 0;

        list_node * product_node = product_ll;
        while(product_node != NULL) {
            position * p = malloc(sizeof(position));
            p->broker = &traders[i-2];
            p->item = product_node->data.product;
            p->value = 0;
            p->quantity = 0;

            list_add(&traders[i-2].positions, p, POSITION);
            product_node = product_node->next;
        }
    }

    return traders;
}

void launch(struct trader * t) {
    printf("%s Starting trader %d (%s)\n", LOG_PREFIX, t->id, t->path);

    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("Fork() in init_traders failed");
        exit(2);
    }

    if (pid == 0) { // child
        //DEBUG

        // THIS WILL SHIT ON >9 traders
        char id = t->id + '0'; 

        char * child_args[] = {t->path, &id, NULL};

		// dup2(STDOUT_FILENO, STDOUT_FILENO);
        execv(child_args[0], child_args);

        perror("execv");
        exit(2);

    } else {
        t->pid = pid;
        t->online = 1;
    }

    return;
}

void init_traders(struct trader * traders, int n) {  
    char fifo_path_trader[PATH_LEN] = {0};
    char fifo_path_exchange[PATH_LEN] = {0};
    
    int trader_id;

    for (int i = 0; i < n; i++) {

        /* Create named pipes */

        trader_id = traders[i].id;

        snprintf(fifo_path_exchange, PATH_LEN, FIFO_EXCHANGE, trader_id);
        snprintf(fifo_path_trader, PATH_LEN, FIFO_TRADER, trader_id);

        if (access(fifo_path_exchange, F_OK) == -1) {
            if (mkfifo(fifo_path_exchange, 0666) == -1) { 
                perror("Exchange mkfifo() failed");
                exit(2);
            }
        }

        printf("%s Created FIFO %s\n", LOG_PREFIX, fifo_path_exchange);        

        if (access(fifo_path_trader, F_OK) == -1) {
            if (mkfifo(fifo_path_trader, 0666) == -1) {
                perror("Trader mkfifo() failed");
                exit(2);
            }
        }

        printf("%s Created FIFO %s\n", LOG_PREFIX, fifo_path_trader);

        /* Execute child binary */ 

        launch(&traders[i]);

        /* Connect to pipes */

        int outgoing_fd = open(fifo_path_exchange, O_WRONLY);
        fcntl(outgoing_fd, F_SETFL, O_NONBLOCK);

        if (outgoing_fd == -1) {
            perror("Open exchange FIFO");
            exit(2);
        }

        printf("%s Connected to %s\n", LOG_PREFIX, fifo_path_exchange);

        int incoming_fd = open(fifo_path_trader, O_RDONLY);
        fcntl(incoming_fd, F_SETFL, O_NONBLOCK);

        if (incoming_fd == -1) {
            perror("Open trader FIFO");
            exit(2);
        } 

        printf("%s Connected to %s\n", LOG_PREFIX, fifo_path_trader);
        traders[trader_id].incoming_fd = incoming_fd;
        traders[trader_id].outgoing_fd = outgoing_fd;
    }
}

void close_fifos(struct trader * t) {
    close(t->incoming_fd);
    close(t->outgoing_fd);
}

/**
 * 
 * FRAMEWORK FOR DATA TRANSFER
 * 
 */

void send_data(int fd, char * message) {
    if (write(fd, message, strlen(message)) == -1) {
        // perror("send_data shit the bed");
        return;
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


/**
 * 
 * 
 * SENDING DATA TO TRADERS
 * 
 * 
 */

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
            status = "CANCELLED";
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

/** 
 * 
 * 
 * REPORTING DATA IN PEX
 * 
 */

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

            printf("%s\t\tSELL %i @ $%i (%i orders)\n", LOG_PREFIX, sum_quantity, o->unit_cost, e);
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

            printf("%s\t\tBUY %i @ $%i (%i orders)\n", LOG_PREFIX, sum_quantity, o->unit_cost, e);
        }
    }
    
}

void print_orderbook(list_node * product_ll) {
    printf("%s\t--ORDERBOOK--\n", LOG_PREFIX);
    
    list_node * cursor = product_ll;
    while(cursor != NULL) {
        int buy_levels = get_levels(cursor->data.product->buy_orders);
        int sell_levels = get_levels(cursor->data.product->sell_orders);

        printf("%s\tProduct: %s; Buy levels: %i; Sell levels: %i\n", 
            LOG_PREFIX, cursor->data.product->name, buy_levels, 
                sell_levels);
        
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

            printf("%s %i ($%lli)", p->item->name, p->quantity, p->value);
            
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

/**
 * 
 * 
 * DATA QUERIES
 * 
 * 
 */

position * find_position(trader * t, product * p) {
    list_node* cursor = t->positions;

    if (cursor == NULL) return NULL;

    while (cursor) {
        if (&cursor->data.position->item->name == &p->name)
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


int get_levels(list_node * order_head) {
    int count = 0;

    list_node * cursor = order_head;
    while (cursor != NULL) {

        if (cursor->prev == NULL) {
            count++;
            cursor=cursor->next;
            continue;
        }

        if (cursor->prev->data.order->unit_cost == cursor->data.order->unit_cost) {
            ; // do nothing
        } else {
            count++;
        }

        cursor = cursor->next;
    }

    return count;
} 


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

        cursor = cursor->next;
    }

    return NULL;
}