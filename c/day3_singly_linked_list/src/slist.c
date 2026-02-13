#include "slist.h"
#include <stdlib.h>

#define OK      (0)
#define REMOVED (1)
#define CYCLE_EXISTS (1)
#define ERR     (-1)

static slist_node_t* node_create(int value) {
    slist_node_t* n = (slist_node_t*)malloc(sizeof(*n));
    if (!n) return NULL;
    n->value = value;
    n->next = NULL;
    return n;
}

int slist_init(slist_t* list)
{
    if (list == NULL) {
        return ERR;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return OK;
}

void slist_free(slist_t* list)
{
    if (list == NULL) {
        return;
    }

    /* Free the nodes if they are present */
    slist_node_t *curr_node = list->head;
    slist_node_t *tmp;

    while(curr_node && list->size) {
        tmp = curr_node;
        curr_node = tmp->next;
        free(tmp);
        list->size--;
    }
    /* TODO: assert list->size == 0 */

    list->head = NULL;
    list->tail = NULL;
}

int slist_push_front(slist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    /* Create new node */
    slist_node_t *new_node = node_create(value);

    if (new_node == NULL) {
        return ERR;
    }

    new_node->next = list->head;
    list->head = new_node;

    if (list->tail == NULL) {
        /* empty slist */
        list->tail = new_node;
    }
    list->size++;

    return OK;
}

int slist_push_back(slist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    /* Create new node */
    slist_node_t *new_node = node_create(value);

    if (new_node == NULL) {
        return ERR;
    }

    if (list->tail == NULL) {
        /* empty list */
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }

    list->size++;
    return OK;
}

int slist_pop_front(slist_t* list, int* out_value)
{
    if (list == NULL ||
        list->head == NULL ||
        list->size == 0 ||
        out_value == NULL) {
        return ERR;
    }

    slist_node_t *front_node = list->head;
    list->head = front_node->next;
    list->size--;

    if (list->size == 0) {
        list->head = NULL;
        list->tail = NULL;
    }

    *out_value = front_node->value;
    free(front_node);

    return OK;
}

int slist_remove_first(slist_t* list, int value)
{
    if (list == NULL) {
        return ERR;
    }

    if (list->size == 0) {
        /* no elements in the list */
        return OK;
    }

    slist_node_t *curr_node = list->head;
    slist_node_t *node_to_free;

    /* verify head */
    if (curr_node->value == value) {

        /* remove and update head */
        node_to_free = curr_node;
        list->head = curr_node->next;
        free(node_to_free);
        list->size--;

        if (list->size == 0) {
            /* no more elements in list */
            list->head = NULL;
            list->tail = NULL;
        }

        return REMOVED;
    }

    /* cycle the rest */
    while (curr_node->next) {
        if (curr_node->next->value == value) {
            /* remove the next node */
            node_to_free = curr_node->next;
            curr_node->next = node_to_free->next;
            if (node_to_free->next == NULL) {
                /* removing the tail, need update list->tail */
                list->tail = curr_node;
            }
            list->size--;
            free(node_to_free);
            return REMOVED;
        } else {
            curr_node = curr_node->next;
        }
    }

    return OK;
}

int slist_reverse(slist_t* list)
{
    if (list == NULL) {
        return ERR;
    }

    slist_node_t *prev = NULL;
    slist_node_t *next = NULL;
    slist_node_t *curr = list->head;

    list->tail = list->head;

    /* swap */
    while (curr) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    list->head = prev;

    return OK;
}

int slist_find_middle(const slist_t* list, int* out_value)
{
    if (list == NULL || out_value == NULL) {
        return ERR;
    }

    if (list->size == 0) {
        /* empty list */
        return ERR;
    }

    if (list->size == 1) {
        /* one element only */
        *out_value = list->head->value;
        return OK;
    }

    slist_node_t *slow = list->head;
    slist_node_t *fast = list->head->next;
    /* TODO: watchout the cycle */

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    *out_value = slow->value;
    return OK;
}

int slist_has_cycle(const slist_t* list)
{
    if (list == NULL) {
        return ERR;
    }

#if (1)
    /* based on fast and slow pointers */

    if (list->size == 1) {
        /* if there is only one element, tail and head should be the same but the tail->next shouldn't */
        return (list->head == list->tail->next)? CYCLE_EXISTS : OK;
    }

    slist_node_t *slow = list->head;
    slist_node_t *fast = list->head;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return CYCLE_EXISTS;
        }
    }
    return OK;
#endif // Floyd's

#if (0)
    /* find the cycle */
    slist_node_t *curr = list->head;
    size_t iter = list->size;
    size_t cycles = 0;

    while (curr) {
        curr = curr->next;
        cycles++;
        if (cycles > iter) {
            return CYCLE_EXISTS;
        }
    }
    return OK;
#endif // My implementation
}
