#include "mm/kheap.h"

#include "mm/paging.h"
#include "mm/vmm.h"
#include "mm/vmm_layout.h"

/* Stage 6B PMM API is currently implemented in stage0_main.c. */
extern int stage6b_pmm_alloc_frame(uint64_t* out_phys_addr);

#define STAGE8C_KHEAP_MIN_ALIGNMENT 8u

struct stage8c_kheap_state {
    uint32_t is_initialized;
    uint32_t start;
    uint32_t current;
    uint32_t end_exclusive;
    uint32_t mapped_end_exclusive;
};

static struct stage8c_kheap_state g_stage8c_kheap;

static int stage8c_kheap_align_up(uint32_t value, uint32_t alignment, uint32_t* out_aligned_value)
{
    const uint32_t mask = alignment - 1u;

    if (out_aligned_value == (uint32_t*)0) {
        return 0;
    }

    if (alignment == 0u || (alignment & mask) != 0u) {
        return 0;
    }

    if ((value & mask) == 0u) {
        *out_aligned_value = value;
        return 1;
    }

    if (value > (UINT32_MAX - mask)) {
        return 0;
    }

    *out_aligned_value = (value + mask) & ~mask;
    return 1;
}

static int stage8c_kheap_ensure_mapped_until(uint32_t required_end_exclusive)
{
    while (g_stage8c_kheap.mapped_end_exclusive < required_end_exclusive) {
        const uint32_t virtual_page_addr = g_stage8c_kheap.mapped_end_exclusive;
        uint64_t physical_frame_addr_64 = 0u;
        uint32_t physical_frame_addr = 0u;

        if (g_stage8c_kheap.mapped_end_exclusive >= g_stage8c_kheap.end_exclusive) {
            return 0;
        }

        if (stage6b_pmm_alloc_frame(&physical_frame_addr_64) == 0) {
            return 0;
        }

        if (physical_frame_addr_64 > (uint64_t)UINT32_MAX) {
            return 0;
        }

        physical_frame_addr = stage7a_paging_frame_addr((uint32_t)physical_frame_addr_64);
        if (stage8b_vmm_map_page(virtual_page_addr, physical_frame_addr) == 0) {
            return 0;
        }

        g_stage8c_kheap.mapped_end_exclusive += STAGE7A_PAGING_PAGE_SIZE;
    }

    return 1;
}

int stage8c_kheap_bootstrap_init(void)
{
    if (g_stage8c_kheap.is_initialized != 0u) {
        return 1;
    }

    if (stage8a_vmm_layout_validate_policy() == 0) {
        return 0;
    }

    g_stage8c_kheap.start = STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START;
    g_stage8c_kheap.current = STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START;
    g_stage8c_kheap.end_exclusive = STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE;
    g_stage8c_kheap.mapped_end_exclusive = STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START;
    g_stage8c_kheap.is_initialized = 1u;

    return 1;
}

void* stage8c_kheap_alloc(uint32_t size)
{
    uint32_t aligned_size = 0u;
    uint32_t allocation_end_exclusive = 0u;
    uint32_t required_mapped_end_exclusive = 0u;
    const uint32_t allocation_start = g_stage8c_kheap.current;

    if (g_stage8c_kheap.is_initialized == 0u || size == 0u) {
        return (void*)0;
    }

    if (stage8c_kheap_align_up(size, STAGE8C_KHEAP_MIN_ALIGNMENT, &aligned_size) == 0) {
        return (void*)0;
    }

    if (aligned_size > (g_stage8c_kheap.end_exclusive - g_stage8c_kheap.current)) {
        return (void*)0;
    }

    allocation_end_exclusive = allocation_start + aligned_size;
    if (stage8c_kheap_align_up(
            allocation_end_exclusive,
            STAGE7A_PAGING_PAGE_SIZE,
            &required_mapped_end_exclusive)
        == 0) {
        return (void*)0;
    }

    if (stage8c_kheap_ensure_mapped_until(required_mapped_end_exclusive) == 0) {
        return (void*)0;
    }

    g_stage8c_kheap.current = allocation_end_exclusive;
    return (void*)(uintptr_t)allocation_start;
}

int stage8c_kheap_get_state(
    uint32_t* out_start,
    uint32_t* out_current,
    uint32_t* out_end_exclusive,
    uint32_t* out_mapped_end_exclusive)
{
    if (g_stage8c_kheap.is_initialized == 0u
        || out_start == (uint32_t*)0
        || out_current == (uint32_t*)0
        || out_end_exclusive == (uint32_t*)0
        || out_mapped_end_exclusive == (uint32_t*)0) {
        return 0;
    }

    *out_start = g_stage8c_kheap.start;
    *out_current = g_stage8c_kheap.current;
    *out_end_exclusive = g_stage8c_kheap.end_exclusive;
    *out_mapped_end_exclusive = g_stage8c_kheap.mapped_end_exclusive;
    return 1;
}