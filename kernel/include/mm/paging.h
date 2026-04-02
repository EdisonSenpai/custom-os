#ifndef A6506C01_6A6A_4F36_9170_E6D284EE55A4
#define A6506C01_6A6A_4F36_9170_E6D284EE55A4
#ifndef CUSTOM_OS_KERNEL_MM_PAGING_H
#define CUSTOM_OS_KERNEL_MM_PAGING_H

#include <stdint.h>

#define STAGE7A_PAGING_PAGE_SHIFT 12u
#define STAGE7A_PAGING_PAGE_SIZE 0x1000u
#define STAGE7A_PAGING_PAGE_OFFSET_MASK 0x00000FFFu
#define STAGE7A_PAGING_FRAME_ADDR_MASK 0xFFFFF000u
#define STAGE7A_PAGING_INDEX_MASK 0x000003FFu
#define STAGE7A_PAGING_PT_INDEX_SHIFT 12u
#define STAGE7A_PAGING_PD_INDEX_SHIFT 22u
#define STAGE7A_PAGING_ENTRIES_PER_TABLE 1024u
#define STAGE7A_PAGING_FLAG_MASK 0x00000FFFu

typedef uint32_t stage7a_pde_t;
typedef uint32_t stage7a_pte_t;

enum stage7a_paging_flags {
    STAGE7A_PAGING_FLAG_PRESENT = 1u << 0,
    STAGE7A_PAGING_FLAG_WRITABLE = 1u << 1,
    STAGE7A_PAGING_FLAG_USER = 1u << 2,
    STAGE7A_PAGING_FLAG_WRITE_THROUGH = 1u << 3,
    STAGE7A_PAGING_FLAG_CACHE_DISABLE = 1u << 4,
    STAGE7A_PAGING_FLAG_ACCESSED = 1u << 5,
    STAGE7A_PAGING_FLAG_DIRTY = 1u << 6,
    STAGE7A_PAGING_FLAG_PAT = 1u << 7,
    STAGE7A_PAGING_FLAG_GLOBAL = 1u << 8,
    STAGE7A_PAGING_FLAG_AVAILABLE_MASK = 7u << 9
};

uint32_t stage7a_paging_pd_index(uint32_t virtual_addr);
uint32_t stage7a_paging_pt_index(uint32_t virtual_addr);
uint32_t stage7a_paging_page_offset(uint32_t virtual_addr);
uint32_t stage7a_paging_frame_addr(uint32_t addr);
uint32_t stage7a_paging_identity_map_addr(uint32_t physical_addr);
uint32_t stage7a_paging_make_entry(uint32_t frame_addr, uint32_t flags);

#endif


#endif /* A6506C01_6A6A_4F36_9170_E6D284EE55A4 */
