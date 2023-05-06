#include "pe_exchange.h"

/** USYD CODE CITATION ACKNOWLEDGEMENT
 * I declare that the following lines of code have been sourced 
 * from Richard McKenzie's lab solution and implemented
 * with some level of modifications
 * 
 * URL
 * https://edstem.org/au/courses/10466/workspaces/pXPFniggiFxocDNyTPzgNF2hw8mA00wk
 * 
 */ 

list_node * list_init(void* data, data_type type) {
    list_node * n = calloc(1, sizeof(list_node));
    n->type = type;

    switch(type) {
        case ORDER:
            n->data.order = *(order*)data;
            break;
        case PRODUCT:
            n->data.product = *(product*)data;
            break;
        case POSITION:
            n->data.position = *(position*)data;
            break;
    }

    n->next = NULL;
    return n;
    
}

list_node * list_next(list_node * n) {
    if (n == NULL) return NULL;
    return n->next;
}

void list_add(list_node** h, void* data, data_type type) {
    if (*h == NULL) {
        *h = list_init(data, type);
        return;
    }

    list_node* cursor = *h;
    while (cursor->next) 
        cursor = cursor->next;

    cursor->next = list_init(data, type);
}

void list_delete(list_node** h, list_node* n) {
    if (*h == NULL) return;
    list_node* cursor = *h;
    if (*h == n) {
        *h = (*h)->next;
        return;
    }
    while (cursor->next != n){
        if (cursor->next == NULL) return;
        cursor = cursor->next;
    }
    list_node* future = cursor->next->next;
    free(cursor->next);
    cursor->next = future;
    return;
}

void list_free(list_node* h) {
    list_node * cursor = h;

    while(cursor) {
        list_node * tmp = cursor->next;
        free(cursor);
        cursor = tmp;
    }
}

list_node* list_find(list_node* h, const char* name) {
    if (h == NULL || name == NULL) return NULL;
    
    list_node* cursor = h;
    while (cursor) {
        if (cursor->type == PRODUCT && strcmp(cursor->data.product.name, name) == 0) {
            return cursor;
        }
        cursor = cursor->next;
    }

    return NULL; // If not found
}
