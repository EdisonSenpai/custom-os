#ifndef C7D86190_5F3A_4A2A_8B73_E351D9B37359
#define C7D86190_5F3A_4A2A_8B73_E351D9B37359
#ifndef CUSTOM_OS_KERNEL_MM_KMALLOC_H
#define CUSTOM_OS_KERNEL_MM_KMALLOC_H

#include <stdint.h>

void* kmalloc(uint32_t size);
int kfree(void* ptr);

#endif


#endif /* C7D86190_5F3A_4A2A_8B73_E351D9B37359 */
