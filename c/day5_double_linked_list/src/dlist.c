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

int dlist_init(dlist_t* list)
{
    if (list == NULL) {
        return ERR;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return OK;
}

void dlist_free(dlist_t* list)
{
    if (list == NULL) {
        return;
    }

    /* cycle the list and free all nodes */
    dlist_node_t *curr = list->head;
    dlist_node_t *tmp = NULL;

    while (curr) {
        tmp = curr;
        curr = curr->next;

        if (list->size) {
            list->size--;
        }

        free(tmp);
    }

    list->head = NULL;
    list->tail = NULL;
    /* TODO: assert list->size == 0 */
}

int dlist_push_front(dlist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    dlist_node_t *new_node = node_create(value);

    if (new_node == NULL) {
        return ERR;
    }

    new_node->prev = NULL;
    new_node->next = list->head;

    if (list->size == 0) {
        /* push first element, need to update tail too */
        list->tail = new_node;
    } else {
        /* head and tail already there */
        list->head->prev = new_node;
    }

    list->head = new_node;
    list->size++;

    return OK;
}

int dlist_push_back(dlist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    dlist_node_t *new_node = node_create(value);

    if (new_node == NULL) {
        return ERR;
    }

    new_node->prev = list->tail;
    new_node->next = NULL;

    if (list->size == 0) {
        /* push first element, need to update head too */
        list->head = new_node;
    } else {
        /* head and tail already there */
        list->tail->next = new_node;
    }

    list->tail = new_node;
    list->size++;

    return OK;
}

int dlist_pop_front(dlist_t* list, int* out_value)
{
    if (list == NULL || out_value == NULL) {
        return ERR;
    }

    dlist_node_t *node_to_free = list->head;

    if (node_to_free == NULL) {
        /* there is no head */
        /* TODO: assert list->size == 0 */
        return ERR;
    }

    /* when list has only one element */
    if (list->size == 1) {
        *out_value = node_to_free->value;
        free(node_to_free);
        if (list->size) {
            list->size--;
        }
        list->head = NULL;
        list->tail = NULL;
        /* TODO: assert list->size == 0*/
        return OK;
    }

    /* when list has more than one element */
    dlist_node_t *new_head = node_to_free->next;

    /* update new head */
    new_head->prev = NULL;
    list->head = new_head;

    /* return the value */
    *out_value = node_to_free->value;

    /* remove the node */
    if (list->size) {
        list->size--;
    }
    free(node_to_free);

    return OK;
}

int dlist_pop_back(dlist_t* list, int* out_value)
{
    if (list == NULL || out_value == NULL) {
        return ERR;
    }

    dlist_node_t *node_to_free = list->tail;

    if (node_to_free == NULL) {
        /* there is no tail */
        /* TODO: assert list->size == 0 */
        return ERR;
    }

    /* when list has only one element */
    if (list->size == 1) {
        *out_value = node_to_free->value;
        free(node_to_free);
        if (list->size) {
            list->size--;
        }
        list->head = NULL;
        list->tail = NULL;
        /* TODO: assert list->size == 0 */
        return OK;
    }

    /* when list has more than one element */
    dlist_node_t *new_tail = node_to_free->prev;

    /* update new head */
    new_tail->next = NULL;
    list->tail = new_tail;

    /* return the value */
    *out_value = node_to_free->value;

    /* remove the node */
    if (list->size) {
        list->size--;
    }
    free(node_to_free);

    return OK;
}

int dlist_remove_first(dlist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    dlist_node_t *curr = list->head;
    dlist_node_t *node_to_free;

    if (curr == NULL) {
        /* no head (size == 0) or corrupt (size != 0) */
        return OK;
    }

    while (curr) {
        if (curr->value == value) {
            /* found first, remove it */
            node_to_free = curr;

            if (node_to_free == list->head) {
                /* we remove the head */
                /* TODO: assert node_to_free->prev == NULL */
                if (node_to_free->next != NULL) {
                    /* the head is not the tail, so there is one more element - update it */
                    node_to_free->next->prev = NULL;
                }
                list->head = node_to_free->next;
            } else {
                node_to_free->prev->next = node_to_free->next;
            }

            if (node_to_free == list->tail) {
                /* we remove the tail */
                /* TODO: assert node_to_free->next == NULL */
                if (node_to_free->prev != NULL) {
                    /* the tail is not the head, so update previous element next pointer */
                    node_to_free->prev->next = NULL;
                }
                list->tail = node_to_free->prev;
            } else {
                node_to_free->next->prev = node_to_free->prev;
            }

            if (list->size) {
                list->size--;
            }

            free(node_to_free);
            return REMOVED;
        } else {
            curr = curr->next;
        }
    }

    return OK;

}
