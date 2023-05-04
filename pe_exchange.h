#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#include <sys/epoll.h>

#define PATH_LEN 50
#define BUFFER_LEN 50

#define LOG_PREFIX "[PEX]"

/** 
 * Database-like structs
 */

typedef struct product_node{
    struct product_node* next;
    char * data;

} product_node;

typedef struct order_node {
    struct order_node* next;

    struct trader* trader;
    int quantity;
    int unit_cost;
    struct product_node * product; 
    int order_id;
} order_node;

struct trader {
    int id;
    int incoming_fd;
    int outgoing_fd;
    char path[PATH_LEN];
    int pid;
};


product_node* product_list_init(char * input);

product_node* product_list_next(product_node* n);

void product_list_add(product_node** h, char * input);

void product_list_free(product_node* head);

void MARKET_OPEN(struct trader * traders, int len);


#endif 