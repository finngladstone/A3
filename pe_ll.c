#include "pe_exchange.h"

/** USYD CODE CITATION ACKNOWLEDGEMENT
 * I declare that the following lines of code have been sourced 
 * from Richard McKenzie's lab solutions and implemented
 * with some level of modifications
 * 
 * URL
 * https://edstem.org/au/courses/10466/workspaces/pXPFniggiFxocDNyTPzgNF2hw8mA00wk
 * 
 */

product_node* product_list_init(char * input) {
    product_node* n = calloc(1, sizeof(product_node));
    n->next = NULL;
    n->data = (char *) malloc(strlen(input) + 1);

    strcpy(n->data, input);

    return n;
}

product_node* product_list_next(product_node* n) {
    if (n == NULL) return NULL;
    return n->next;
}

void product_list_add(product_node** h, char * input) {
    if (*h == NULL) {
        *h = product_list_init(input);
        return;
    }
    product_node* cursor = *h;
    while (cursor->next){
        cursor = cursor->next;
    }
    cursor->next = product_list_init(input);
}

void product_list_free(product_node* head) {
    product_node* cursor = head;
    while(cursor){
        product_node* tmp = cursor->next;
        free(cursor->data);
        free(cursor);
        cursor = tmp;
    }
}