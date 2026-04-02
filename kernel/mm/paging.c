#include "mm/paging.h"

_Static_assert(STAGE7A_PAGING_PAGE_SIZE == 4096u, "Stage 7A requires 4 KiB pages");
_Static_assert(STAGE7A_PAGING_ENTRIES_PER_TABLE == 1024u, "Stage 7A requires 1024-entry directories/tables");

static page_directory_t g_stage7b_early_page_directory;
static page_table_t g_stage7b_early_identity_page_table;

static void stage7b_clear_page_table(page_table_t* table)
{
    uint32_t i = 0u;

    while (i < STAGE7A_PAGING_ENTRIES_PER_TABLE) {
        table->entries[i] = 0u;
        i++;
    }
}

static void stage7b_clear_page_directory(page_directory_t* directory)
{
    uint32_t i = 0u;

    while (i < STAGE7A_PAGING_ENTRIES_PER_TABLE) {
        directory->entries[i] = 0u;
        i++;
    }
}

uint32_t stage7a_paging_pd_index(uint32_t virtual_addr)
{
    return (virtual_addr >> STAGE7A_PAGING_PD_INDEX_SHIFT) & STAGE7A_PAGING_INDEX_MASK;
}

uint32_t stage7a_paging_pt_index(uint32_t virtual_addr)
{
    return (virtual_addr >> STAGE7A_PAGING_PT_INDEX_SHIFT) & STAGE7A_PAGING_INDEX_MASK;
}

uint32_t stage7a_paging_page_offset(uint32_t virtual_addr)
{
    return virtual_addr & STAGE7A_PAGING_PAGE_OFFSET_MASK;
}

uint32_t stage7a_paging_frame_addr(uint32_t addr)
{
    return addr & STAGE7A_PAGING_FRAME_ADDR_MASK;
}

uint32_t stage7a_paging_identity_map_addr(uint32_t physical_addr)
{
    return stage7a_paging_frame_addr(physical_addr);
}

uint32_t stage7a_paging_make_entry(uint32_t frame_addr, uint32_t flags)
{
    const uint32_t masked_frame = stage7a_paging_frame_addr(frame_addr);
    const uint32_t masked_flags = flags & STAGE7A_PAGING_FLAG_MASK;

    return masked_frame | masked_flags;
}

void stage7b_setup_early_identity_paging(void)
{
    const uint32_t base_flags = STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE;
    const uint32_t identity_table_frame = stage7a_paging_frame_addr((uint32_t)(uintptr_t)&g_stage7b_early_identity_page_table);
    uint32_t i = 0u;

    stage7b_clear_page_directory(&g_stage7b_early_page_directory);
    stage7b_clear_page_table(&g_stage7b_early_identity_page_table);

    while (i < STAGE7A_PAGING_ENTRIES_PER_TABLE) {
        const uint32_t frame_addr = i << STAGE7A_PAGING_PAGE_SHIFT;
        g_stage7b_early_identity_page_table.entries[i] = stage7a_paging_make_entry(frame_addr, base_flags);
        i++;
    }

    g_stage7b_early_page_directory.entries[0] = stage7a_paging_make_entry(identity_table_frame, base_flags);
}

const page_directory_t* stage7b_get_early_page_directory(void)
{
    return &g_stage7b_early_page_directory;
}

const page_table_t* stage7b_get_early_identity_page_table(void)
{
    return &g_stage7b_early_identity_page_table;
}

void stage7c_load_cr3_with_early_page_directory(void)
{
    const uint32_t page_directory_frame =
        stage7a_paging_frame_addr((uint32_t)(uintptr_t)stage7b_get_early_page_directory());

    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory_frame) : "memory");
}

void stage7c_set_cr0_paging_enable(void)
{
    const uint32_t updated_cr0 = stage7c_read_cr0() | STAGE7C_CR0_PG_MASK;

    __asm__ volatile (
        "mov %0, %%cr0\n\t"
        "jmp 1f\n"
        "1:\n"
        :
        : "r"(updated_cr0)
        : "memory");
}

uint32_t stage7c_read_cr0(void)
{
    uint32_t cr0 = 0u;

    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

uint32_t stage7c_read_cr3(void)
{
    uint32_t cr3 = 0u;

    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}