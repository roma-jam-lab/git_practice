#ifndef EVENT_NOTIFIER_H
#define EVENT_NOTIFIER_H

#include <stdlib.h>
#include <stdbool.h>

#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback used to notify subscriber
 */
typedef struct
{
    // Create fields of this struct as you need.
    int dummy;
}Event;

/**
 * @brief Callback used to notify subscriber
 *
 * @param[out] Actual event pointer
 * @param[out] Pointer to an associated data
 * @param[out] Data length
 */
typedef void (*event_cb_t)(const Event *event, const void *data, size_t length);

void event_initialize(Event* event);
void event_deinitialize(Event* event);
bool event_subscribe(Event* event, void (*handler)(const Event*, const void*, size_t));
bool event_unsubscribe(Event* event, void (*handler)(const Event*, const void*, size_t));
void event_notify(Event* event, const void* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif // EVENT_NOTIFIER_H
