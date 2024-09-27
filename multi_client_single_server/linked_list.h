/* Define a simple double Linked List*/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

// -- Struct definitions
typedef struct Node
{
    // Unique string key to help track nodes
    char* key;
    // We hold a pointer to whatever variable or stuct we use as data
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct LinkedList
{
    int count;
    struct Node *first;
    struct Node *last;
} LinkedList;
// --

// -- Function declarations
int AddNode(LinkedList *list, Node *node, int index);
int DelNodeByAddress(LinkedList *list, Node *nodeAddress);
int DelNodeByKey(LinkedList *list, char *nodeKey);
// --

#endif /* LINKED_LIST_H */