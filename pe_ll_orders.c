// #include "pe_exchange.h"

// node * list_init()

struct order_node {
    struct order_node* next;
    
    int trader_id;
    int quantity;
    int unit_cost;

};