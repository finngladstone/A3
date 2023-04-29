#ifndef PE_EXCHANGE_H
#define PE_EXCHANGE_H

#include "pe_common.h"

#define PATH_LEN 50
#define BUFFER_LEN 128

#define LOG_PREFIX "[PEX]"

typedef struct node{
    struct node* next;
    char * data;
} node;

node* list_init(char * input);

node* list_next(node* n);

void list_add(node** h, char * input);

void list_free(node* head);


#endif 