#include "dlist.h"
#include <stdlib.h>

#define OK  (0)
#define ERR (-1)
#define REMOVED (1)

static dlist_node_t* node_create(int value) {
    dlist_node_t* n = (dlist_node_t*)malloc(sizeof(*n));
    if (!n) return NULL;
    n->value = value;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

int dlist_init(dlist_t* list) {
    /* TODO */
    (void)list;
    return ERR;
}

void dlist_free(dlist_t* list) {
    /* TODO */
    (void)list;
}

int dlist_push_front(dlist_t* list, int value) {
    /* TODO */
    (void)list; (void)value;
    return ERR;
}

int dlist_push_back(dlist_t* list, int value) {
    /* TODO */
    (void)list; (void)value;
    return ERR;
}

int dlist_pop_front(dlist_t* list, int* out_value) {
    /* TODO */
    (void)list; (void)out_value;
    return ERR;
}

int dlist_pop_back(dlist_t* list, int* out_value) {
    /* TODO */
    (void)list; (void)out_value;
    return ERR;
}

int dlist_remove_first(dlist_t* list, int value) {
    /* TODO */
    (void)list; (void)value;
    return ERR;
}
