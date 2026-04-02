#include <stdint.h>

#include "mm/vmm_layout.h"

_Static_assert(STAGE8A_VMM_LAYOUT_PAGE_SIZE_4K == 0x1000u, "Stage 8A requires 4 KiB policy alignment");
_Static_assert((STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A active identity start must be 4 KiB aligned");
_Static_assert((STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A active identity end must be 4 KiB aligned");
_Static_assert((STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A future heap start must be 4 KiB aligned");
_Static_assert((STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A future heap end must be 4 KiB aligned");
_Static_assert((STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A future mapping start must be 4 KiB aligned");
_Static_assert((STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u, "Stage 8A future mapping end must be 4 KiB aligned");

_Static_assert(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START < STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE, "Stage 8A active identity range must be non-empty");
_Static_assert(STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START < STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE, "Stage 8A future heap range must be non-empty");
_Static_assert(STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START < STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE, "Stage 8A future mapping range must be non-empty");
_Static_assert(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE <= STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START, "Stage 8A active identity and future heap ranges must not overlap");
_Static_assert(STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE <= STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START, "Stage 8A future heap and future mapping ranges must not overlap");
_Static_assert(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START == 0x00000000u, "Stage 8A active identity range must start at 0");
_Static_assert(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE == 0x00400000u, "Stage 8A active identity range must cover first 4 MiB");

static int stage8a_vmm_layout_addr_in_half_open_range(uint32_t addr, uint32_t range_start, uint32_t range_end_exclusive)
{
    return (addr >= range_start) && (addr < range_end_exclusive);
}

static int stage8a_vmm_layout_compute_region_end(uint32_t start_addr, uint32_t length, uint32_t* out_end_exclusive)
{
    uint64_t end = 0u;

    if (out_end_exclusive == (uint32_t*)0 || length == 0u) {
        return 0;
    }

    end = (uint64_t)start_addr + (uint64_t)length;

    if (end > (uint64_t)UINT32_MAX) {
        return 0;
    }

    *out_end_exclusive = (uint32_t)end;
    return *out_end_exclusive > start_addr;
}

static int stage8a_vmm_layout_region_in_half_open_range(
    uint32_t start_addr,
    uint32_t length,
    uint32_t range_start,
    uint32_t range_end_exclusive)
{
    uint32_t end_exclusive = 0u;

    if (stage8a_vmm_layout_compute_region_end(start_addr, length, &end_exclusive) == 0) {
        return 0;
    }

    return (start_addr >= range_start) && (end_exclusive <= range_end_exclusive);
}

int stage8a_vmm_layout_is_addr_in_active_identity(uint32_t virtual_addr)
{
    return stage8a_vmm_layout_addr_in_half_open_range(
        virtual_addr,
        STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START,
        STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE);
}

int stage8a_vmm_layout_is_addr_in_future_heap_reserved(uint32_t virtual_addr)
{
    return stage8a_vmm_layout_addr_in_half_open_range(
        virtual_addr,
        STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START,
        STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE);
}

int stage8a_vmm_layout_is_addr_in_future_mapping_reserved(uint32_t virtual_addr)
{
    return stage8a_vmm_layout_addr_in_half_open_range(
        virtual_addr,
        STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START,
        STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE);
}

int stage8a_vmm_layout_is_region_in_active_identity(uint32_t start_addr, uint32_t length)
{
    return stage8a_vmm_layout_region_in_half_open_range(
        start_addr,
        length,
        STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START,
        STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE);
}

int stage8a_vmm_layout_is_region_in_future_heap_reserved(uint32_t start_addr, uint32_t length)
{
    return stage8a_vmm_layout_region_in_half_open_range(
        start_addr,
        length,
        STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START,
        STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE);
}

int stage8a_vmm_layout_is_region_in_future_mapping_reserved(uint32_t start_addr, uint32_t length)
{
    return stage8a_vmm_layout_region_in_half_open_range(
        start_addr,
        length,
        STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START,
        STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE);
}

stage8a_vmm_layout_region_class_t stage8a_vmm_layout_classify_addr(uint32_t virtual_addr)
{
    if (stage8a_vmm_layout_is_addr_in_active_identity(virtual_addr) != 0) {
        return STAGE8A_VMM_LAYOUT_REGION_ACTIVE_IDENTITY;
    }

    if (stage8a_vmm_layout_is_addr_in_future_heap_reserved(virtual_addr) != 0) {
        return STAGE8A_VMM_LAYOUT_REGION_FUTURE_HEAP_RESERVED;
    }

    if (stage8a_vmm_layout_is_addr_in_future_mapping_reserved(virtual_addr) != 0) {
        return STAGE8A_VMM_LAYOUT_REGION_FUTURE_MAPPING_RESERVED;
    }

    return STAGE8A_VMM_LAYOUT_REGION_UNCLASSIFIED;
}

const char* stage8a_vmm_layout_region_class_name(stage8a_vmm_layout_region_class_t region_class)
{
    switch (region_class) {
    case STAGE8A_VMM_LAYOUT_REGION_ACTIVE_IDENTITY:
        return "active-identity";
    case STAGE8A_VMM_LAYOUT_REGION_FUTURE_HEAP_RESERVED:
        return "future-heap-reserved";
    case STAGE8A_VMM_LAYOUT_REGION_FUTURE_MAPPING_RESERVED:
        return "future-mapping-reserved";
    case STAGE8A_VMM_LAYOUT_REGION_UNCLASSIFIED:
        return "unclassified";
    default:
        return "unknown";
    }
}

int stage8a_vmm_layout_validate_policy(void)
{
    if (STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START) == 0u
        || STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE) == 0u
        || STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START) == 0u
        || STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE) == 0u
        || STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START) == 0u
        || STAGE8A_VMM_LAYOUT_IS_4K_ALIGNED(STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE) == 0u) {
        return 0;
    }

    if (STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START != 0x00000000u
        || STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE != 0x00400000u) {
        return 0;
    }

    if (STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START >= STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE
        || STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START >= STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE
        || STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START >= STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE) {
        return 0;
    }

    if (STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE > STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START
        || STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE > STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START) {
        return 0;
    }

    return 1;
}