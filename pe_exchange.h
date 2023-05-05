#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#include <sys/epoll.h>

/** Constants */

#define PATH_LEN 50
#define BUFFER_LEN 50

#define LOG_PREFIX "[PEX]"


/** Forward definitions */

typedef struct position position;
typedef struct product product;
typedef struct order order;
typedef struct trader trader;

/** 
 * Database-like structs
*/

typedef struct trader {
    int id;
    int incoming_fd;
    int outgoing_fd;

    char path[PATH_LEN];
    int pid;

    order * orders; //ll
    position * positions; //ll

} trader;

/** These structs will be used in LLs */

typedef struct product {
    order * buy_orders;
    order * sell_orders;

    char name[BUFFER_LEN];

} product;

typedef struct order {
    trader broker;
    product product;

    int quantity;
    int unit_cost;
    int order_id;

} order;

typedef struct position {
    trader broker;
    product item;
    int value; 
} position;

typedef union {
    product product;
    position position;
    order order;
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
    struct list_node* next;
    data_type type;
} list_node;

/** LINKED LIST BACKEND  */

list_node* list_init(void* data, data_type type);
list_node * list_next(list_node * n);
void list_add(list_node** h, void* data, data_type type);
void list_delete(list_node** h, list_node* n);
void list_free(list_node* h);

/** Helper functions */
trader * find_trader(int pid, struct trader * traders, int n);
char* id_command(char * src);

#endif 