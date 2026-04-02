#include "mm/vmm.h"

#include "mm/paging.h"
#include "mm/vmm_layout.h"

#define STAGE8B_VMM_SINGLE_TABLE_SPAN (STAGE7A_PAGING_ENTRIES_PER_TABLE * STAGE7A_PAGING_PAGE_SIZE)
#define STAGE8B_VMM_MUTABLE_RESERVED_START STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START
#define STAGE8B_VMM_MUTABLE_RESERVED_END_EXCLUSIVE STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE
#define STAGE8B_VMM_MUTABLE_RESERVED_SPAN \
    (STAGE8B_VMM_MUTABLE_RESERVED_END_EXCLUSIVE - STAGE8B_VMM_MUTABLE_RESERVED_START)
#define STAGE8B_VMM_MUTABLE_TABLE_COUNT (STAGE8B_VMM_MUTABLE_RESERVED_SPAN / STAGE8B_VMM_SINGLE_TABLE_SPAN)

_Static_assert((STAGE8B_VMM_MUTABLE_RESERVED_SPAN % STAGE8B_VMM_SINGLE_TABLE_SPAN) == 0u,
    "Stage 8B mutable reserved span must be an exact multiple of one page-table span");
_Static_assert(STAGE8B_VMM_MUTABLE_TABLE_COUNT == 3u,
    "Stage 8B/8C expects three page tables for 0x00400000-0x01000000 mutable windows");

static page_table_t g_stage8b_mutable_reserved_page_tables[STAGE8B_VMM_MUTABLE_TABLE_COUNT];

static int stage8b_vmm_is_page_aligned(uint32_t addr)
{
    return stage7a_paging_page_offset(addr) == 0u;
}

static void stage8b_vmm_invalidate_page(uint32_t virtual_page_addr)
{
    const void* page = (const void*)(uintptr_t)virtual_page_addr;

    __asm__ volatile ("invlpg (%0)" : : "r"(page) : "memory");
}

static page_table_t* stage8b_vmm_page_table_from_entry(uint32_t entry)
{
    return (page_table_t*)(uintptr_t)stage7a_paging_frame_addr(entry);
}

static void stage8b_vmm_clear_page_table(page_table_t* page_table)
{
    uint32_t i = 0u;

    while (i < STAGE7A_PAGING_ENTRIES_PER_TABLE) {
        page_table->entries[i] = 0u;
        i++;
    }
}

static int stage8b_vmm_table_is_empty(const page_table_t* page_table)
{
    uint32_t i = 0u;

    while (i < STAGE7A_PAGING_ENTRIES_PER_TABLE) {
        if ((page_table->entries[i] & STAGE7A_PAGING_FLAG_PRESENT) != 0u) {
            return 0;
        }

        i++;
    }

    return 1;
}

static int stage8b_vmm_get_reserved_mapping_slot(
    uint32_t virtual_page_addr,
    uint32_t* out_pd_index,
    uint32_t* out_table_slot)
{
    const uint32_t reserved_start_pd_index =
        stage7a_paging_pd_index(STAGE8B_VMM_MUTABLE_RESERVED_START);
    const uint32_t pd_index = stage7a_paging_pd_index(virtual_page_addr);
    uint32_t table_slot = 0u;

    if (stage8a_vmm_layout_is_addr_in_future_heap_reserved(virtual_page_addr) == 0
        && stage8a_vmm_layout_is_addr_in_future_mapping_reserved(virtual_page_addr) == 0) {
        return 0;
    }

    if (pd_index < reserved_start_pd_index) {
        return 0;
    }

    table_slot = pd_index - reserved_start_pd_index;
    if (table_slot >= STAGE8B_VMM_MUTABLE_TABLE_COUNT) {
        return 0;
    }

    if (out_pd_index != (uint32_t*)0) {
        *out_pd_index = pd_index;
    }

    if (out_table_slot != (uint32_t*)0) {
        *out_table_slot = table_slot;
    }

    return 1;
}

int stage8b_vmm_is_page_mapped(uint32_t virtual_page_addr)
{
    const page_directory_t* page_directory = stage7b_get_early_page_directory();
    const uint32_t pd_index = stage7a_paging_pd_index(virtual_page_addr);
    const uint32_t pt_index = stage7a_paging_pt_index(virtual_page_addr);
    const uint32_t pde = page_directory->entries[pd_index];
    const page_table_t* page_table = 0;

    if (stage8b_vmm_is_page_aligned(virtual_page_addr) == 0) {
        return 0;
    }

    if ((pde & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        return 0;
    }

    page_table = stage8b_vmm_page_table_from_entry(pde);
    return (page_table->entries[pt_index] & STAGE7A_PAGING_FLAG_PRESENT) != 0u;
}

int stage8b_vmm_resolve_page_frame(uint32_t virtual_page_addr, uint32_t* out_physical_frame_addr)
{
    const page_directory_t* page_directory = stage7b_get_early_page_directory();
    const uint32_t pd_index = stage7a_paging_pd_index(virtual_page_addr);
    const uint32_t pt_index = stage7a_paging_pt_index(virtual_page_addr);
    const uint32_t pde = page_directory->entries[pd_index];
    const page_table_t* page_table = 0;
    const uint32_t* pte = 0;

    if (out_physical_frame_addr == (uint32_t*)0) {
        return 0;
    }

    if (stage8b_vmm_is_page_aligned(virtual_page_addr) == 0) {
        return 0;
    }

    if ((pde & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        return 0;
    }

    page_table = stage8b_vmm_page_table_from_entry(pde);
    pte = &page_table->entries[pt_index];
    if ((*pte & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        return 0;
    }

    *out_physical_frame_addr = stage7a_paging_frame_addr(*pte);
    return 1;
}

int stage8b_vmm_map_page(uint32_t virtual_page_addr, uint32_t physical_frame_addr)
{
    page_directory_t* page_directory = stage7b_get_early_page_directory_mutable();
    const uint32_t mapping_flags = STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE;
    uint32_t pd_index = 0u;
    uint32_t table_slot = 0u;
    page_table_t* expected_page_table = 0;
    uint32_t pde = 0u;
    uint32_t pt_index = 0u;

    if (stage8b_vmm_is_page_aligned(virtual_page_addr) == 0) {
        return 0;
    }

    if (stage8b_vmm_is_page_aligned(physical_frame_addr) == 0) {
        return 0;
    }

    if (stage8b_vmm_get_reserved_mapping_slot(virtual_page_addr, &pd_index, &table_slot) == 0) {
        return 0;
    }

    expected_page_table = &g_stage8b_mutable_reserved_page_tables[table_slot];
    pde = page_directory->entries[pd_index];

    if ((pde & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        stage8b_vmm_clear_page_table(expected_page_table);
        page_directory->entries[pd_index] = stage7a_paging_make_entry(
            (uint32_t)(uintptr_t)expected_page_table,
            mapping_flags);
    } else if (stage7a_paging_frame_addr(pde)
        != stage7a_paging_frame_addr((uint32_t)(uintptr_t)expected_page_table)) {
        return 0;
    }

    pt_index = stage7a_paging_pt_index(virtual_page_addr);
    if ((expected_page_table->entries[pt_index] & STAGE7A_PAGING_FLAG_PRESENT) != 0u) {
        return 0;
    }

    expected_page_table->entries[pt_index] = stage7a_paging_make_entry(physical_frame_addr, mapping_flags);
    stage8b_vmm_invalidate_page(virtual_page_addr);
    return 1;
}

int stage8b_vmm_unmap_page(uint32_t virtual_page_addr)
{
    page_directory_t* page_directory = stage7b_get_early_page_directory_mutable();
    uint32_t pd_index = 0u;
    uint32_t table_slot = 0u;
    page_table_t* expected_page_table = 0;
    uint32_t pde = 0u;
    uint32_t pt_index = 0u;

    if (stage8b_vmm_is_page_aligned(virtual_page_addr) == 0) {
        return 0;
    }

    if (stage8b_vmm_get_reserved_mapping_slot(virtual_page_addr, &pd_index, &table_slot) == 0) {
        return 0;
    }

    expected_page_table = &g_stage8b_mutable_reserved_page_tables[table_slot];
    pde = page_directory->entries[pd_index];
    if ((pde & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        return 0;
    }

    if (stage7a_paging_frame_addr(pde)
        != stage7a_paging_frame_addr((uint32_t)(uintptr_t)expected_page_table)) {
        return 0;
    }

    pt_index = stage7a_paging_pt_index(virtual_page_addr);
    if ((expected_page_table->entries[pt_index] & STAGE7A_PAGING_FLAG_PRESENT) == 0u) {
        return 0;
    }

    expected_page_table->entries[pt_index] = 0u;
    stage8b_vmm_invalidate_page(virtual_page_addr);

    if (stage8b_vmm_table_is_empty(expected_page_table) != 0) {
        page_directory->entries[pd_index] = 0u;
    }

    return 1;
}