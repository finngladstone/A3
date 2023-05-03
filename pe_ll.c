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

node* list_init(char * input) {
    node* n = calloc(1, sizeof(node));
    n->next = NULL;
    n->data = (char *) malloc(strlen(input) + 1);

    strcpy(n->data, input);

    return n;
}

node* list_next(node* n) {
    if (n == NULL) return NULL;
    return n->next;
}

void list_add(node** h, char * input) {
    if (*h == NULL) {
        *h = list_init(input);
        return;
    }
    node* cursor = *h;
    while (cursor->next){
        cursor = cursor->next;
    }
    cursor->next = list_init(input);
}

void list_free(node* head) {
    node* cursor = head;
    while(cursor){
        node* tmp = cursor->next;
        free(cursor->data);
        free(cursor);
        cursor = tmp;
    }
}