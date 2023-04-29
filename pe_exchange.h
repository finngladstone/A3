#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#include <sys/epoll.h>

#define PATH_LEN 50
#define BUFFER_LEN 50

#define LOG_PREFIX "[PEX]"

typedef struct node{
    struct node* next;
    char * data;
} node;

typedef struct trader {
    int id;
    int incoming_fd;
    int outgoing_fd;
    char path[PATH_LEN];
} trader;

node* list_init(char * input);

node* list_next(node* n);

void list_add(node** h, char * input);

void list_free(node* head);


#endif 