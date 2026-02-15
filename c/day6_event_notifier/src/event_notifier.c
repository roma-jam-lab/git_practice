#include <sys/queue.h>

#include "event_notifier.h"

#include <stdio.h>

/*
* Event notifier
* Implement an event notifier routine which will allow users to subscribe, unsubscribe and call an
* event.
*
* [] Implement the function  which initializes the event specific  struct.
* [] Implement the functions  and  which accept an event specific struct, event handler which is
    subscribing or unsubscribing that event, and return  which indicates whether
    subscription/unsubscription was successful or not.
* [] Implement the function  which accepts an event specific struct, pointer to a data and the data
    length to pass to the subscribers' handlers.
* [] Implement the function  which releases all the resources acquired for the specified event.
*
* Event handler
* [] An event handler is a function which does not return any value and accepts a pointer to the
    actual event, associated data, and the data length.
*
* HINTS:
* Please make sure to avoid any memory leaks when subscriptions and unsubscriptions are made.
* There is no need to validate the inputs for a NULL pointer value.
* The user should be smart enough not to pass them.
*
* All the subscribed handlers:
* [] have to be called in the subscription order (need to preserve the order)
* [] In case of multiple subscriptions of the same handler, it should be called multiple times in the
*   right order.
* [] When unsubscription is made of a handler that was inserted multiple times,
*   only its first occurrence (the oldest one) should be removed.
*/

/* Implementation:
* Preserve subscription order -> SLIST
* Several subscription in order -> SLIST
* When unsubscribed, remove_first() -> SLIST
* No requirements for O(1) on subscribe/removal -> SLIST for simplicity sake
*/

/* TODO: multithread access */
/* Didn't implement it to keep some time, but added macro to use it in the code */
#define LOCK()
#define UNLOCK()

/**
 * @brief Subscriber node
 */
struct subscriber_s {
    SLIST_ENTRY(subscriber_s) next;
    event_cb_t event_cb;
};

SLIST_HEAD(subscriber_list_s, subscriber_s);

/**
 * @brief Event list node
 */
struct event_node_s
{
    struct {
        SLIST_ENTRY(event_node_s) next_node;
        struct subscriber_list_s subs_list;
        size_t subs_num;
    } dynamic;
    Event *event; /* pointer to event */
};

typedef struct event_node_s *event_node_t;

SLIST_HEAD(events_list_s, event_node_s);

/** @brief List of event nodes, contain event and list of subscribers */
struct event_registry {
    struct events_list_s events_list;
    size_t num;
};

/* one internal event registry exemplar */
struct event_registry s_events = {
    .num = 0,
};

static struct event_node_s *event_create_new(Event* event) {
    struct event_node_s *new_node = (struct event_node_s*)calloc(1, sizeof(struct event_node_s));
    if (!new_node) {
        return NULL;
    }
    /* save the params and init subs list */
    new_node->event = event;
    SLIST_INIT(&new_node->dynamic.subs_list);
    return new_node;
}

static struct subscriber_s *subscriber_create_new(event_cb_t subscriber_cb) {
    struct subscriber_s* new_subscriber = (struct subscriber_s*)calloc(1, sizeof(struct subscriber_s));
    if (!new_subscriber) {
        return NULL;
    }
    new_subscriber->event_cb = subscriber_cb;
    return new_subscriber;
}

void event_initialize(Event* event)
{
    /* If not events were made -> init events registry */
    if (s_events.num == 0) {
        SLIST_INIT(&s_events.events_list);
    } else {
        /* TODO: init on non empty */
    }

    /* Save event in registry */
    struct event_node_s *new_event_node = event_create_new(event);
    if (!new_event_node) {
        /* TODO: assert? return? */
        return;
    }

    LOCK();

    SLIST_INSERT_HEAD(&s_events.events_list, new_event_node, dynamic.next_node);
    s_events.num++;

    UNLOCK();
}

void event_deinitialize(Event* event)
{
    /* TODO: what to do on non-empty? */

    /* Cycle the SLIST, found the node, remove the subscription and the node itself */
    LOCK();

    struct event_node_s *event_node = NULL;
    struct event_node_s *tmp_node;
    SLIST_FOREACH(tmp_node, &s_events.events_list, dynamic.next_node) {
        if (tmp_node->event == event) {
            /* found the event in the registry */
            event_node = tmp_node;
            break;
        };
    }

    /* Event node found */

    /* TODO: Unsubscribe all? In lock? Yes.
    * But there might be an "event removed" callback.
    */

    if (event_node) {
        SLIST_REMOVE(&s_events.events_list, event_node, event_node_s, dynamic.next_node);
        /* TODO: assert on num == 0 */
        s_events.num--;

    }

    UNLOCK();

    free(event_node);
}

bool event_subscribe(Event* event, void (*handler)(const Event*, const void*, size_t))
{
    /* Cycle the SLIST, found the node, add the subscription_cb */
    LOCK();

    struct event_node_s *event_node = NULL;
    struct event_node_s *tmp_node;
    SLIST_FOREACH(tmp_node, &s_events.events_list, dynamic.next_node) {
        if (tmp_node->event == event) {
            /* found the event in the registry */
            event_node = tmp_node;
        };
    }

    UNLOCK();

    if (event_node == NULL) {
        /* event wasn't registered */
        return false;
    }

    struct subscriber_s* new_subscriber = subscriber_create_new(handler);
    if (!new_subscriber) {
        return false;
    }

    /* TODO: seek the event + add subscriber in one transaction? */
    LOCK();

    SLIST_INSERT_HEAD(&event_node->dynamic.subs_list, new_subscriber, next);
    event_node->dynamic.subs_num++;

    UNLOCK();

    return true;
}

bool event_unsubscribe(Event* event, void (*handler)(const Event*, const void*, size_t))
{
    /* Cycle the SLIST, found the node, add the subscription_cb */
    LOCK();

    struct event_node_s *event_node = NULL;
    struct event_node_s *tmp_node;
    SLIST_FOREACH(tmp_node, &s_events.events_list, dynamic.next_node) {
        if (tmp_node->event == event) {
            /* found the event in the registry */
            event_node = tmp_node;
        };
    }

    UNLOCK();

    if (event_node == NULL) {
        /* event wasn't registered */
        return false;
    }

    /* Seek the subscriber via callback */
    LOCK();

    struct subscriber_s *subscriber_to_free = NULL;
    struct subscriber_s *tmp_subs;
    SLIST_FOREACH(tmp_subs, &event_node->dynamic.subs_list, next) {
        if (tmp_subs->event_cb == handler) {
            /* found the subscriber in subscribers event list */
            subscriber_to_free = tmp_subs;
            break;
        };
    }

    if (subscriber_to_free) {
        SLIST_REMOVE(&event_node->dynamic.subs_list, subscriber_to_free, subscriber_s, next);
        /* TODO: assert on num == 0 */
        event_node->dynamic.subs_num--;
    }

    UNLOCK();

    /* TODO: add flag for print correct message */
    if (subscriber_to_free) {
        free(subscriber_to_free);
        return true;
    }

    return true;
}

void event_notify(Event* event, const void* data, size_t length)
{
    /* Cycle the SLIST, found the node, call the callback */
}
