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
            n->data.order = (order*)data;
            break;
        case PRODUCT:
            n->data.product = (product*)data;
            break;
        case POSITION:
            n->data.position = (position*)data;
            break;
    }

    n->next = NULL;
    n->prev = NULL;
    return n;
}


list_node * list_next(list_node * n) {
    if (n == NULL) return NULL;
    return n->next;
}

list_node * list_prev(list_node * n) { // Add this function
    if (n == NULL) return NULL;
    return n->prev;
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
    cursor->next->prev = cursor; // Add this line
}

void list_add_sorted(list_node** h, void* data, data_type type) { // for order only
    if (*h == NULL) {
        *h = list_init(data, type);
        return;
    }
    
    list_node * new_node = list_init(data, type);
    
    list_node* cursor = *h;
    list_node* prev = NULL;

    int new_price = 0;
    int cursor_price = 0;

    while (cursor != NULL) {
        new_price = ((order*)data)->unit_cost;
        cursor_price = cursor->data.order->unit_cost;

        if (new_price <= cursor_price) {
            break;
        }

        prev = cursor;
        cursor = cursor->next;
    }
    
    if (prev == NULL) {
        new_node->next = *h;
        (*h)->prev = new_node;
        *h = new_node;
    } else {
        new_node->next = prev->next;
        new_node->prev = prev;
        if (prev->next != NULL) {
            prev->next->prev = new_node;
        }
        prev->next = new_node;
    }
}

void list_delete(list_node** h, list_node* n) {
    if (*h == NULL) return;
    list_node* cursor = *h;
    if (*h == n) {
        *h = (*h)->next;
        if (*h) (*h)->prev = NULL; // Add this line
        return;
    }
    while (cursor->next != n){
        if (cursor->next == NULL) return;
        cursor = cursor->next;
    }
    list_node* future = cursor->next->next;
    if (future) future->prev = cursor; // Add this line
    free(cursor->next);
    cursor->next = future;
    return;
}


void list_free(list_node* head) {
    list_node* current = head;
    list_node* temp;
    
    while (current != NULL) {
        temp = current;
        current = current->next;
        
        if (temp->type == PRODUCT) {
            // Free the memory allocated for the product structs
            free(temp->data.product);
        }

        // Free the memory allocated for the list_node
        free(temp);
    }
}

list_node* list_find(list_node* h, const char* name) {
    if (h == NULL || name == NULL) return NULL;
    
    list_node* cursor = h;
    while (cursor) {
        if (cursor->type == PRODUCT && strcmp(cursor->data.product->name, name) == 0) {
            return cursor;
        }
        cursor = cursor->next;
    }

    return NULL; // If not found
}

list_node* list_get_tail(list_node* h) {
    if (h == NULL) {
        return NULL;
    }

    list_node* cursor = h;
    while (cursor->next != NULL) {
        cursor = cursor->next;
    }

    return cursor;
}