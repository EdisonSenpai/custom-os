#include "mm/kmalloc.h"

#include "mm/kheap.h"

void* kmalloc(uint32_t size)
{
    return stage8c_kheap_alloc(size);
}

int kfree(void* ptr)
{
    return stage9a_kheap_free(ptr);
}
