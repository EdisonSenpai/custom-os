#ifndef A49A95C04_5CFF_4A1A_A66C_BFD8B610A9FA
#define A49A95C04_5CFF_4A1A_A66C_BFD8B610A9FA
#ifndef CUSTOM_OS_KERNEL_MM_KMEMORY_H
#define CUSTOM_OS_KERNEL_MM_KMEMORY_H

#include <stdint.h>

void* kmemset(void* dest, int value, uint32_t count);
void* kmemcpy(void* dest, const void* src, uint32_t count);
void* kmemmove(void* dest, const void* src, uint32_t count);
int kmemcmp(const void* a, const void* b, uint32_t count);

#endif


#endif /* A49A95C04_5CFF_4A1A_A66C_BFD8B610A9FA */
