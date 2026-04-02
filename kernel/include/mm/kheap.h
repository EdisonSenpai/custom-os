#ifndef D8252296_6C45_468A_9E45_060C553FC777
#define D8252296_6C45_468A_9E45_060C553FC777
#ifndef CUSTOM_OS_KERNEL_MM_KHEAP_H
#define CUSTOM_OS_KERNEL_MM_KHEAP_H

#include <stdint.h>

int stage8c_kheap_bootstrap_init(void);
void* stage8c_kheap_alloc(uint32_t size);
int stage9a_kheap_free(void* ptr);
int stage8c_kheap_get_state(
    uint32_t* out_start,
    uint32_t* out_current,
    uint32_t* out_end_exclusive,
    uint32_t* out_mapped_end_exclusive);

#endif


#endif /* D8252296_6C45_468A_9E45_060C553FC777 */
