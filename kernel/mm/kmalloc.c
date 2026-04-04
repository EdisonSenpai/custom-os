#include "mm/kmalloc.h"

#include "mm/kheap.h"

void* kmalloc(uint32_t size)
{
    if (size == 0u) {
        return (void*)0;
    }

    return stage8c_kheap_alloc(size);
}

int kfree(void* ptr)
{
    if (ptr == (void*)0) {
        return 0;
    }

    return stage9a_kheap_free(ptr);
}
