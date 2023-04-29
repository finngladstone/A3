#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the structure for a singly linked list node
typedef struct Node {
    char* data;
    struct Node* next;
} Node;

// Function to create a new node with the given data
Node* createNode(const char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = (char*)malloc(strlen(data) + 1);
    strcpy(newNode->data, data);
    newNode->next = NULL;
    return newNode;
}

// Function to insert a node at the end of the list
void insertEnd(Node** head, const char* data) {
    Node* newNode = createNode(data);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// Function to display the list
void displayList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("%s -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

// Function to free the list
void freeList(Node** head) {
    Node* current = *head;
    Node* nextNode;
    while (current != NULL) {
        nextNode = current->next;
        free(current->data); // Free the dynamically allocated string
        free(current);
        current = nextNode;
    }
    *head = NULL;
}

int main() {
    Node* head = NULL;
    
    insertEnd(&head, "apple");
    insertEnd(&head, "banana");
    insertEnd(&head, "cherry");
    insertEnd(&head, "date");

    printf("List: ");
    displayList(head);

    freeList(&head);

    return 0;
}
