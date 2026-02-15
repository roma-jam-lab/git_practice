#include <cstdlib>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
void* __real_malloc(size_t s)
{
    return malloc(s);
}
void* __real_calloc(size_t s, size_t n)
{
    return calloc(s, n);
}
void* __real_realloc(void* ptr, size_t s)
{
    return realloc(ptr, s);
}
void __real_free(void* ptr)
{
    free(ptr);
}

#ifdef __cplusplus
}
#endif
