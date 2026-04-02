#include "mm/paging.h"

_Static_assert(STAGE7A_PAGING_PAGE_SIZE == 4096u, "Stage 7A requires 4 KiB pages");
_Static_assert(STAGE7A_PAGING_ENTRIES_PER_TABLE == 1024u, "Stage 7A requires 1024-entry directories/tables");

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