#include "dyn_array.h"
#include <stdlib.h>
#include <string.h>

#define OK      (0)
#define ERR    (-1)

#define LOCK()
#define UNLOCK()

int dyn_array_init(dyn_array_t* arr, size_t initial_capacity)
{
    if (arr == NULL || initial_capacity == 0) {
        return ERR;
    }

    int* mem_chunk = calloc(initial_capacity, sizeof(int));

    if (mem_chunk == NULL) {
        /* Unable to allocate memory */
        return ERR;
    }

    LOCK();
    arr->data = mem_chunk;
    arr->capacity = initial_capacity;
    arr->size = 0;
    UNLOCK();

    return OK;
}

void dyn_array_free(dyn_array_t* arr)
{
    if (arr == NULL) {
        return;
    }

    int* mem_chunk = NULL;

    LOCK();
    if (arr->data) {
        mem_chunk = arr->data;
        arr->data = NULL;
        arr->capacity = 0;
        arr->size = 0;
    }
    UNLOCK();

    if (mem_chunk) {
        free(mem_chunk);
    }
}

int dyn_array_push_back(dyn_array_t* arr, int value)
{
    if (arr == NULL) {
        return ERR;
    }

    int ret;
    LOCK();
    if (arr->size < arr->capacity) {
        arr->data[arr->size++] = value;
        ret = OK;
    } else {
        /* Add one more */
        size_t new_capacity = arr->capacity + 1;
        int *old_chunk = arr->data;
        int *new_chunk = calloc(new_capacity, sizeof(int));
        if (new_chunk == NULL) {
            ret = ERR;
        } else {
            memcpy(new_chunk, old_chunk, arr->size);
            arr->data = new_chunk;
            arr->data[arr->size++] = value;
            arr->capacity = new_capacity;
            free(old_chunk);
            ret = OK;
        }
    }
    UNLOCK();

    return ret;
}

int dyn_array_pop_back(dyn_array_t* arr)
{
    if (arr == NULL) {
        return ERR;
    }

    int ret;
    LOCK();
    if (arr->data && (arr->size > 0)) {
        arr->size--;
        arr->data[arr->size] = 0;
        ret = OK;
    } else {
        ret = ERR;
    }
    UNLOCK();

    return ret;
}

int dyn_array_get(const dyn_array_t* arr, size_t index, int* out_value)
{
    if (arr == NULL || out_value == NULL) {
        return ERR;
    }

    int ret;
    int value = 0;

    LOCK();
    if (index < arr->size) {
        value = arr->data[index];
        ret = OK;
    } else {
        ret = ERR;
    }
    UNLOCK();

    if (ret == OK) {
        *out_value = value;
    }
    return ret;
}

int dyn_array_resize(dyn_array_t* arr, size_t new_size)
{
    if (arr == NULL) {
        return ERR;
    }

    /* TODO: Lock? */
    size_t total_size = arr->size + new_size;

    if (total_size < new_size) {
        /* Overflow */
        return ERR;
    }

    if (total_size <= arr->capacity) {
        /* extend current */
        arr->size += new_size;
    } else {
        /* re-alloc capacity */
        int *mem_chunk = calloc(total_size, sizeof(int));
        int *old_mem_chunk = NULL;

        if (mem_chunk == NULL) {
            return ERR;
        }

        LOCK();
        old_mem_chunk = arr->data;
        memcpy(mem_chunk, old_mem_chunk, arr->size * sizeof(int));
        arr->size += new_size;
        arr->data = mem_chunk;
        arr->capacity = total_size;
        UNLOCK();

        free(old_mem_chunk);
    }

    return OK;
}
