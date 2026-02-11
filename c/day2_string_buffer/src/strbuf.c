#include "strbuf.h"
#include <stdlib.h>
#include <string.h>

#define OK  (0)
#define ERR (-1)

static const char kEmptyStr[] = "";

// static int ensure_capacity(strbuf_t* sb, size_t min_capacity)
// {
//     /* TODO: grow sb->capacity to at least min_capacity bytes */
//     /* min_capacity counts bytes including the required trailing '\0'. */
//     (void)sb; (void)min_capacity;
//     return ERR;
// }

int strbuf_init(strbuf_t* sb, size_t initial_capacity)
{
    char *mem_chunk;
    int ret;

    if (sb == NULL) {
        return ERR;
    }

    if (initial_capacity == 0) {
        /* lazy alloc */
        mem_chunk = NULL;
        ret = OK;
    } else {
        /* alloc memory */
        mem_chunk = calloc(initial_capacity, sizeof(char));
        if (mem_chunk == NULL) {
            initial_capacity = 0;
            ret = ERR;
        } else {
            ret = OK;
        }
    }

    sb->data = mem_chunk;
    sb->size = 0;
    sb->capacity = initial_capacity;

    return ret;
}

void strbuf_free(strbuf_t* sb)
{
    if (sb == NULL) {
        return;
    }

    char *mem_chunk = sb->data;

    sb->data = NULL;
    sb->size = 0;
    sb->capacity = 0;

    free(mem_chunk);
}

const char* strbuf_c_str(const strbuf_t* sb)
{
    if (sb == NULL) {
        return kEmptyStr;
    }

    if (sb->data && sb->size) {
        return (const char*) &sb->data[0];
    } else {
        return kEmptyStr;
    }
}

int strbuf_append_cstr(strbuf_t* sb, const char* suffix)
{
    if (sb == NULL || suffix == NULL) {
        return ERR;
    }

    size_t suffix_len = strlen(suffix);
    size_t vacant_len = sb->capacity - sb->size - 1;

    if (suffix_len == 0) {
        /* suffix is '\0' */
        return OK;
    }

    if (sb->data && suffix_len <= vacant_len) {
        /* we have place in memory - store and return */
        memcpy(&sb->data[sb->size], suffix, suffix_len);
        sb->size += suffix_len;
        /* terminate with NUL */
        sb->data[sb->size] = '\0';
        return OK;
    }

    /* need to grow or lazy alloc */
    /* TODO: watchout the sb->capacity overflow */
    size_t new_capacity = sb->capacity + suffix_len;

    char *old_mem_chunk = sb->data;
    char *new_mem_chunk = calloc(new_capacity, sizeof(char));

    if (new_mem_chunk == NULL) {
        return ERR;
    }

    if (sb->size == 0 && old_mem_chunk == NULL) {
        /* lazy alloc */
        new_capacity++;
    } else {
        memcpy(new_mem_chunk, old_mem_chunk, sb->size);
    }
    memcpy(new_mem_chunk + sb->size, suffix, suffix_len);
    sb->data = new_mem_chunk;
    sb->size += suffix_len;
    sb->capacity = new_capacity;
    /* termination */
    sb->data[sb->size] = '\0';

    if (old_mem_chunk != NULL) {
        free(old_mem_chunk);
    }
    return OK;
}

int strbuf_append_n(strbuf_t* sb, const char* s, size_t n)
{
    if (sb == NULL || (s == NULL && n > 0)) {
        return ERR;
    }

    size_t suffix_len = n;
    size_t vacant_len = sb->capacity - sb->size - 1;

    if (suffix_len == 0) {
        return OK;
    }

    if (sb->data && suffix_len <= vacant_len) {
        /* we have place in memory - store and return */
        memcpy(&sb->data[sb->size], s, suffix_len);
        sb->size += suffix_len;
        /* terminate with NUL */
        sb->data[sb->size] = '\0';
        return OK;
    }

    /* need to grow or lazy alloc */
    /* TODO: watchout the sb->capacity overflow */
    size_t new_capacity = sb->capacity + suffix_len;

    char *old_mem_chunk = sb->data;
    char *new_mem_chunk = calloc(new_capacity, sizeof(char));

    if (new_mem_chunk == NULL) {
        return ERR;
    }

    if (sb->size == 0 && old_mem_chunk == NULL) {
        /* lazy alloc */
        new_capacity++;
    } else {
        memcpy(new_mem_chunk, old_mem_chunk, sb->size);
    }
    memcpy(new_mem_chunk + sb->size, s, suffix_len);
    sb->data = new_mem_chunk;
    sb->size += suffix_len;
    sb->capacity = new_capacity;
    /* termination */
    sb->data[sb->size] = '\0';

    if (old_mem_chunk != NULL) {
        free(old_mem_chunk);
    }
    return OK;
}

void strbuf_clear(strbuf_t* sb)
{
    if (sb == NULL) {
        return;
    }

    sb->size = 0;

    if (sb->data) {
        /* not lazy alloc */
        sb->data[sb->size] = '\0';
    }
}
