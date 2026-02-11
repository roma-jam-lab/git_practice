#ifndef STRBUF_H
#define STRBUF_H

#include <stddef.h>

typedef struct {
    char*  data;     // NUL-terminated
    size_t size;     // number of characters excluding the final '\0'
    size_t capacity; // allocated bytes in data (including space for '\0')
} strbuf_t;

/* Initialize with initial_capacity bytes (including space for '\0').
 * If initial_capacity == 0, initialize as empty with capacity 0 (lazy alloc).
 * Returns 0 on success, -1 on allocation failure or invalid args.
 */
int strbuf_init(strbuf_t* sb, size_t initial_capacity);

/* Free resources; safe to call multiple times. */
void strbuf_free(strbuf_t* sb);

/* Return internal C-string pointer.
 * Must never return NULL; return "" for empty/unallocated buffers.
 */
const char* strbuf_c_str(const strbuf_t* sb);

/* Append a C-string (excluding its '\0').
 * Returns 0 on success, -1 on allocation failure or invalid args.
 */
int strbuf_append_cstr(strbuf_t* sb, const char* suffix);

/* Append exactly n bytes from s (bytes may include '\0'; treat as raw bytes).
 * Still ensure the buffer remains NUL-terminated at sb->data[sb->size].
 */
int strbuf_append_n(strbuf_t* sb, const char* s, size_t n);

/* Clear content to empty string (size=0, keep capacity).
 * After clear, c_str() must be "" (i.e., first char is '\0').
 */
void strbuf_clear(strbuf_t* sb);

#endif
