#include "mm/kheap.h"

#include "mm/paging.h"
#include "mm/vmm.h"
#include "mm/vmm_layout.h"

/* Stage 6B PMM API is currently implemented in stage0_main.c. */
extern int stage6b_pmm_alloc_frame(uint64_t* out_phys_addr);

#define STAGE8C_KHEAP_MIN_ALIGNMENT 8u
#define STAGE9A_KHEAP_ALLOCATION_HEADER_MAGIC 0x39414C48u
#define STAGE9A_KHEAP_ALLOCATION_STATE_ALLOCATED 1u
#define STAGE9A_KHEAP_ALLOCATION_STATE_FREED 2u

struct stage9a_kheap_allocation_header {
    uint32_t magic;
    uint32_t payload_size;
    uint32_t state;
    uint32_t reserved;
};

typedef char stage9a_kheap_allocation_header_size_must_be_16[
    (sizeof(struct stage9a_kheap_allocation_header) == 16u) ? 1 : -1];

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
    uint32_t payload_start = 0u;
    uint32_t allocation_end_exclusive = 0u;
    uint32_t required_mapped_end_exclusive = 0u;
    const uint32_t allocation_header_start = g_stage8c_kheap.current;
    struct stage9a_kheap_allocation_header* allocation_header =
        (struct stage9a_kheap_allocation_header*)(uintptr_t)0;

    if (g_stage8c_kheap.is_initialized == 0u || size == 0u) {
        return (void*)0;
    }

    if (stage8c_kheap_align_up(size, STAGE8C_KHEAP_MIN_ALIGNMENT, &aligned_size) == 0) {
        return (void*)0;
    }

    if (allocation_header_start
        > (g_stage8c_kheap.end_exclusive - (uint32_t)sizeof(struct stage9a_kheap_allocation_header))) {
        return (void*)0;
    }

    payload_start = allocation_header_start + (uint32_t)sizeof(struct stage9a_kheap_allocation_header);

    if (aligned_size > (g_stage8c_kheap.end_exclusive - payload_start)) {
        return (void*)0;
    }

    allocation_end_exclusive = payload_start + aligned_size;
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

    allocation_header =
        (struct stage9a_kheap_allocation_header*)(uintptr_t)allocation_header_start;
    allocation_header->magic = STAGE9A_KHEAP_ALLOCATION_HEADER_MAGIC;
    allocation_header->payload_size = aligned_size;
    allocation_header->state = STAGE9A_KHEAP_ALLOCATION_STATE_ALLOCATED;
    allocation_header->reserved = 0u;

    g_stage8c_kheap.current = allocation_end_exclusive;
    return (void*)(uintptr_t)payload_start;
}

int stage9a_kheap_free(void* ptr)
{
    uint32_t cursor = 0u;
    uint32_t target_ptr = 0u;

    if (g_stage8c_kheap.is_initialized == 0u || ptr == (void*)0) {
        return 0;
    }

    target_ptr = (uint32_t)(uintptr_t)ptr;
    if (target_ptr < g_stage8c_kheap.start || target_ptr >= g_stage8c_kheap.current) {
        return 0;
    }

    cursor = g_stage8c_kheap.start;
    while (cursor < g_stage8c_kheap.current) {
        struct stage9a_kheap_allocation_header* header =
            (struct stage9a_kheap_allocation_header*)(uintptr_t)cursor;
        uint32_t payload_start = 0u;
        uint32_t payload_end_exclusive = 0u;

        if (cursor > (g_stage8c_kheap.current - (uint32_t)sizeof(struct stage9a_kheap_allocation_header))) {
            return 0;
        }

        if (header->magic != STAGE9A_KHEAP_ALLOCATION_HEADER_MAGIC) {
            return 0;
        }

        if (header->payload_size == 0u || (header->payload_size & (STAGE8C_KHEAP_MIN_ALIGNMENT - 1u)) != 0u) {
            return 0;
        }

        payload_start = cursor + (uint32_t)sizeof(struct stage9a_kheap_allocation_header);

        if (header->payload_size > (g_stage8c_kheap.current - payload_start)) {
            return 0;
        }

        payload_end_exclusive = payload_start + header->payload_size;

        if (target_ptr == payload_start) {
            if (header->state != STAGE9A_KHEAP_ALLOCATION_STATE_ALLOCATED) {
                return 0;
            }

            header->state = STAGE9A_KHEAP_ALLOCATION_STATE_FREED;
            return 1;
        }

        if (target_ptr > payload_start && target_ptr < payload_end_exclusive) {
            return 0;
        }

        if (payload_end_exclusive <= cursor) {
            return 0;
        }

        cursor = payload_end_exclusive;
    }

    return 0;
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