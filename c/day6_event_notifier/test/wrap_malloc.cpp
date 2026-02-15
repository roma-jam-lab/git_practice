#include <stddef.h>
#include <list>

#include "gtest/gtest.h"

std::list<void*> allocatedData;

#ifdef __cplusplus
extern "C"
{
#endif
extern void* __real_malloc(size_t);
extern void* __real_calloc(size_t, size_t);
extern void* __real_realloc(void*, size_t);
extern void __real_free(void*);

void* __wrap_malloc(size_t size)
{
    void* ptr = __real_malloc(size);
    if(ptr != nullptr)
        allocatedData.push_back(ptr);

    return ptr;
}
void* __wrap_calloc(size_t num, size_t size)
{
    void* ptr = __real_calloc(num, size);
    if(ptr != nullptr)
        allocatedData.push_back(ptr);

    return ptr;
}
void* __wrap_realloc(void* ptr, size_t size)
{
    void* newPtr = __real_realloc(ptr, size);
    if(ptr != nullptr && (size == 0 || newPtr != nullptr))
    {
        auto it = std::find(allocatedData.begin(), allocatedData.end(), ptr);
        EXPECT_TRUE(it != allocatedData.end()) << "Freed data not found in the list. Address: " << ptr;
        allocatedData.erase(it);
    }
    if(newPtr != nullptr)
    {
        allocatedData.push_back(newPtr);
    }

    return newPtr;
}
void __wrap_free(void* ptr)
{
    __real_free(ptr);
    if(ptr != nullptr)
    {
        auto it = std::find(allocatedData.begin(), allocatedData.end(), ptr);
        EXPECT_TRUE(it != allocatedData.end()) << "Freed data not found in the list. Address: " << ptr;
        allocatedData.erase(it);
    }
}
#ifdef __cplusplus
}
#endif
