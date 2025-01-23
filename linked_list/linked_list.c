/* Define a simple double Linked List*/

#include <stddef.h>
#include "linked_list.h"

/*
Add a node to the Linked List.
If index is less than zero or greater than or equal to the current list count, then add the new node at the end of the
current list. Otherwise node will be inserted at the specified index.
Returns the new count of the list.
*/
int AddNode(LinkedList *list, Node *node)
{
    if (list->last == NULL) // List is empty
    {
        list->first = node;
        list->last = node;
        list->count = 1;
    }
    else // put new node at end of list
    {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
        list->count++;
    }

    return list->count;
}

/*
Deletes the node with address matching nodeAdress.
Returns the new count of list if a matching node was found, otherwise returns -1.
*/
int DelNodeByAddress(LinkedList *list, Node *nodeAddress)
{
    int i;
    Node *currentNode = list->first;

    for (i = 0; i < list->count; i++)
    {
        if (currentNode == nodeAddress)
        {
            if (currentNode->prev != NULL)
                currentNode->prev->next = currentNode->next;
            else
                list->first = currentNode->next;
            if (currentNode->next != NULL)
                currentNode->next->prev = currentNode->prev;
            else
                list->last = currentNode->prev;
            list->count--;
            free(currentNode);
            return list->count;
        }

        currentNode = currentNode->next;
    }

    return -1;
}

/*
Deletes the node with key matching nodeKey.
Returns the new count of list if a matching node was found, otherwise returns -1.
*/
int DelNodeByKey(LinkedList *list, char *nodeKey)
{
    int i;
    Node *currentNode = list->first;

    for (i = 0; i < list->count; i++)
    {
        if (strcmp(nodeKey, currentNode->key) == 0)
        {
            if (currentNode->prev != NULL)
                currentNode->prev->next = currentNode->next;
            if (currentNode->next != NULL)
                currentNode->next->prev = currentNode->prev;
            list->count--;
            free(currentNode);
            return list->count;
        }

        currentNode = currentNode->next;
    }

    return -1;
}