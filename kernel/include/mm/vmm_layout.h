#ifndef D9C4E882_F630_4CA9_B204_AE7133705400
#define D9C4E882_F630_4CA9_B204_AE7133705400
#ifndef CUSTOM_OS_KERNEL_MM_VMM_LAYOUT_H
#define CUSTOM_OS_KERNEL_MM_VMM_LAYOUT_H

#include <stdint.h>

#define STAGE8A_VMM_LAYOUT_PAGE_SIZE_4K 0x1000u
#define STAGE8A_VMM_LAYOUT_PAGE_MASK_4K (STAGE8A_VMM_LAYOUT_PAGE_SIZE_4K - 1u)

/* Active Stage 7 baseline: first 4 MiB identity mapped. */
#define STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START 0x00000000u
#define STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE 0x00400000u

/* Reserved policy windows for future work; not mapped/activated in Stage 8A. */
#define STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START 0x00400000u
#define STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE 0x00800000u
#define STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START 0x00800000u
#define STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE 0x01000000u

#define STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END (STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE - 1u)
#define STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END (STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE - 1u)
#define STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END (STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE - 1u)

#define STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(value) (((value) & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u)

typedef enum stage8a_vmm_layout_region_class {
    STAGE8A_VMM_LAYOUT_REGION_ACTIVE_IDENTITY = 0u,
    STAGE8A_VMM_LAYOUT_REGION_FUTURE_HEAP_RESERVED = 1u,
    STAGE8A_VMM_LAYOUT_REGION_FUTURE_MAPPING_RESERVED = 2u,
    STAGE8A_VMM_LAYOUT_REGION_UNCLASSIFIED = 3u
} stage8a_vmm_layout_region_class_t;

int stage8a_vmm_layout_is_addr_in_active_identity(uint32_t virtual_addr);
int stage8a_vmm_layout_is_addr_in_future_heap_reserved(uint32_t virtual_addr);
int stage8a_vmm_layout_is_addr_in_future_mapping_reserved(uint32_t virtual_addr);

int stage8a_vmm_layout_is_region_in_active_identity(uint32_t start_addr, uint32_t length);
int stage8a_vmm_layout_is_region_in_future_heap_reserved(uint32_t start_addr, uint32_t length);
int stage8a_vmm_layout_is_region_in_future_mapping_reserved(uint32_t start_addr, uint32_t length);

stage8a_vmm_layout_region_class_t stage8a_vmm_layout_classify_addr(uint32_t virtual_addr);
const char* stage8a_vmm_layout_region_class_name(stage8a_vmm_layout_region_class_t region_class);

int stage8a_vmm_layout_validate_policy(void);

#endif


#endif /* D9C4E882_F630_4CA9_B204_AE7133705400 */
