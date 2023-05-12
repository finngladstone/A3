#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#include <sys/epoll.h>

/** Constants */

#define PATH_LEN 50
#define BUFFER_LEN 100
#define CMD_LEN 7
#define PRODUCT_LEN 17

#define LOG_PREFIX "[PEX]"


/** Forward definitions */

typedef struct position position;
typedef struct product product;
typedef struct order order;
typedef struct trader trader;
typedef struct list_node list_node;

typedef enum {
    BUY,
    SELL,
    CANCEL
} order_type;

typedef enum {
    BUY_CLEAR,
    SELL_CLEAR,
    BOTH 
} orders_fulfilled;

/** 
 * Database-like structs
*/

typedef struct trader {
    int id;
    int incoming_fd;
    int outgoing_fd;

    char path[PATH_LEN];
    int pid;

    list_node * orders; //ll
    list_node * positions; //ll
    int next_order_id;

    int online;

} trader;

/** These structs will be used in LLs */

typedef struct product {
    list_node * buy_orders;
    list_node * sell_orders;

    char name[PRODUCT_LEN];

} product;

typedef struct order {
    trader * broker;
    product * product;

    int quantity;
    int unit_cost;
    int order_id;

    int time;
    order_type type;

} order;

typedef struct position {
    trader * broker;
    product * item;
    int value; 
    int quantity;
} position;

typedef union {
    product* product;
    position* position;
    order* order;
} node_data;

typedef enum {
    PRODUCT,
    POSITION,
    ORDER
} data_type;

/** 
 * Concept borrowed from
 * https://stackoverflow.com/questions/28107867/different-structs-in-the-same-node-linked-lists-c
*/

typedef struct list_node {
    node_data data;
    struct list_node* prev;
    struct list_node* next;
    data_type type;
} list_node;

/** LINKED LIST BACKEND  */

list_node* list_init(void* data, data_type type);
list_node * list_next(list_node * n);
void list_add(list_node** h, void* data, data_type type);
void list_delete_recursive(list_node** h, list_node* n);
void list_free_recursive(list_node* h);
list_node* list_find(list_node* h, const char* name);
list_node* list_get_tail(list_node* h);
int list_get_len(list_node * h);
list_node* list_get_head(list_node* node);

list_node* list_add_sorted_asc(list_node * head, void* data, data_type type);
list_node* list_add_sorted_desc(list_node * head, void* data, data_type type);
list_node * find_product_order_listnode(product * p, order * o);

void list_free_node(list_node* head);
void list_delete_node_only(list_node** h, list_node* n);
/** Helper functions */

typedef enum {
    ACCEPTED,
    AMENDED,
    CANCELLED,
    INVALID
} statuses;


trader * find_trader(int pid, struct trader * traders, int n);
int id_command(char * src, char * dest);
order * find_trader_order(trader * t, int order_id);
list_node * find_order_listnode(trader * t, int id);
position * find_position(trader * t, product * p);
void spx_report(list_node * product_ll, trader * traders, int n_traders);
int number_of_live_traders(trader * traders, int n);
void print_positions(trader * traders, int n);
void print_orderbook(list_node * product_ll);
void print_aggregate_orders(product * p);


/** Comms */

void send_data(int fd, char * message);
void receive_data(int fd, char * buffer);

void SEND_STATUS(trader * t, int id, statuses s);
void SEND_MARKET_OPEN(trader * traders, int n);
void SEND_MARKET_UPDATE(trader * traders, int n, order o, trader * except);
void SEND_FILL(trader * t, order * o, int quantity);

/** Etc */


#endif 