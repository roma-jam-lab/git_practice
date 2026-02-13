#ifndef DLIST_H
#define DLIST_H

#include <stddef.h>

typedef struct dlist_node {
    int value;
    struct dlist_node* next;
    struct dlist_node* prev;
} dlist_node_t;

typedef struct {
    dlist_node_t* head;
    dlist_node_t* tail;
    size_t size;
} dlist_t;

int dlist_init(dlist_t* list);
void dlist_free(dlist_t* list);

int dlist_push_front(dlist_t* list, int value);
int dlist_push_back(dlist_t* list, int value);

int dlist_pop_front(dlist_t* list, int* out_value);
int dlist_pop_back(dlist_t* list, int* out_value);

int dlist_remove_first(dlist_t* list, int value);

#endif
