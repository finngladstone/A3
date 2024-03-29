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

list_node * list_prev(list_node * n) {
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
    cursor->next->prev = cursor;
}


list_node* list_add_sorted_asc(list_node * head, void* data, data_type type) {
    list_node * new = list_init(data, type);
    int new_c = new->data.order->unit_cost;

    if (head == NULL) {
        return new;
    }

    else if (head->data.order->unit_cost >= new_c) {
        new->next = head;
        new->next->prev = new;
        return new;
    } else {
        list_node * cursor = head;
        while (cursor->next != NULL && 
            cursor->next->data.order->unit_cost < new_c) 
        {
            cursor = cursor->next;
        }

        new->next = cursor->next;
        if (cursor->next != NULL)
            new->next->prev = new;

        cursor->next = new;
        new->prev = cursor;
    }

    return head;
    
} // for order only

list_node* list_add_sorted_desc(list_node * head, void* data, data_type type) {
    list_node * new = list_init(data, type);
    int new_c = new->data.order->unit_cost;

    if (head == NULL) {
        return new;
    }

    else if (head->data.order->unit_cost < new_c) {
        new->next = head;
        new->next->prev = new;
        return new;
    } else {
        list_node * cursor = head;
        while (cursor->next != NULL && 
            cursor->next->data.order->unit_cost >= new_c)
        {
            cursor = cursor->next;
        }

        new->next = cursor->next;
        if (cursor->next != NULL)
            new->next->prev = new;

        cursor->next = new;
        new->prev = cursor;
    }
    
    return head; 
}





void list_delete_recursive(list_node** h, list_node* n) {

    if (*h == NULL) return;

    list_node* cursor = *h;
    if (*h == n) {
        *h = (*h)->next;
        
        switch(n->type) {
            case PRODUCT:
                free(n->data.product);
                break;
            case POSITION:
                free(n->data.position);
                break;
            case ORDER:
                free(n->data.order);
                break;
        }
        free(n);
        
        
        if (*h) (*h)->prev = NULL;
        return;
    }
    while (cursor->next != n){
        if (cursor->next == NULL) return;
        cursor = cursor->next;
    }
    list_node* future = cursor->next->next;
    if (future) future->prev = cursor;

    list_node * tmp = cursor->next;
    switch(tmp->type) {
        case PRODUCT:
            free(tmp->data.product);
            break;
        
        case POSITION:
            free(tmp->data.position);
            break;

        case ORDER:
            free(tmp->data.order);
            break;
    }

    free(cursor->next);
    cursor->next = future;
    return;
}

void list_delete_node_only(list_node** h, list_node* n) {

    if (*h == NULL) return;

    list_node* cursor = *h;
    if (*h == n) {
        *h = (*h)->next;
        if (*h) (*h)->prev = NULL;
        free(n); 
        return;
    }

    while (cursor->next != n){
        if (cursor->next == NULL) return;
        cursor = cursor->next;
    }

    list_node* future = cursor->next->next;
    if (future) future->prev = cursor;

    free(cursor->next); 
    cursor->next = future;

    return;
}



void list_free_recursive(list_node* head) {
    list_node* current = head;
    list_node* temp;
    
    while (current != NULL) {
        temp = current;
        current = current->next;
        
        if (temp->type == PRODUCT) {
            
            list_free_recursive(temp->data.product->buy_orders); 
            list_free_recursive(temp->data.product->sell_orders);
            free(temp->data.product);

        } 

        else if (temp->type == POSITION) {
            free(temp->data.position);

        } else if (temp->type == ORDER) {

            free(temp->data.order);

        }

        free(temp);
    }
}

void list_free_node(list_node* head) {
    list_node* cursor = head;
    while(cursor){
        list_node* tmp = cursor->next;

        if (cursor->type == PRODUCT) {
            list_free_node(cursor->data.product->buy_orders);
            list_free_node(cursor->data.product->sell_orders);
            free(cursor->data.product);
        }

        if (cursor->type == ORDER) {
            // free(cursor->data.order);
        }

        free(cursor);
        cursor = tmp;
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

int list_get_len(list_node * h) {
    int len = 0;
    
    list_node * cursor = h;
    while (cursor != NULL) {
        len++;
        cursor = cursor->next;
    }

    return len;
}

list_node* list_get_head(list_node* node) {
    if (node == NULL) return NULL;

    list_node* cursor = node;
    while (cursor->prev != NULL) {
        cursor = cursor->prev;
    }

    return cursor;
}

/** 
 * 
 * Data queries on LLs 
 * 
*/

list_node * find_product_order_listnode(product * p, order * o) {
    list_node * cursor;
    if (o->type == BUY)
        cursor = p->buy_orders;
    else 
        cursor = p->sell_orders;

    while (cursor != NULL) {
        if (cursor->data.order->order_id == o->order_id) {
            return cursor;
        }

        cursor = cursor->next;
    }

    return NULL;
}

list_node * find_order_listnode(trader * t, int id) {
    list_node* cursor = t->orders;

    while(cursor != NULL) {
        if (cursor->data.order->order_id == id) {
            return cursor;
        }

        cursor = cursor->next;
    }

    return NULL;
}