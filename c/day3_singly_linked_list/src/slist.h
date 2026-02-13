#ifndef SLIST_H
#define SLIST_H

#include <stddef.h>

typedef struct slist_node {
    int value;
    struct slist_node* next;
} slist_node_t;

typedef struct {
    slist_node_t* head;
    slist_node_t* tail;
    size_t size;
} slist_t;

/* Initialize list to empty. Returns 0 on success, -1 on invalid args. */
int slist_init(slist_t* list);

/* Free all nodes and reset list. Safe to call multiple times. */
void slist_free(slist_t* list);

/* Push value at front/back. Returns 0 on success, -1 on allocation failure/invalid args. */
int slist_push_front(slist_t* list, int value);
int slist_push_back(slist_t* list, int value);

/* Pop value from front.
 * Returns 0 on success and writes popped value to *out_value.
 * Returns -1 if list is empty or invalid args.
 */
int slist_pop_front(slist_t* list, int* out_value);

/* Remove first node with matching value.
 * Returns 1 if removed, 0 if not found, -1 on invalid args.
 */
int slist_remove_first(slist_t* list, int value);

/* Reverse the list in-place.
 * Returns 0 on success, -1 on invalid args.
 */
int slist_reverse(slist_t* list);

/* Find the "middle" element.
 * For odd length: exact middle.
 * For even length: return the lower middle (e.g., [1,2,3,4] -> 2).
 * Returns 0 on success (writes to *out_value), -1 on empty list/invalid args.
 */
int slist_find_middle(const slist_t* list, int* out_value);

/* Detect if the list contains a cycle.
 * Returns 1 if cycle exists, 0 if no cycle, -1 on invalid args.
 */
int slist_has_cycle(const slist_t* list);

#endif
