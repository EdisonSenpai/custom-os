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
#define STAGE7C_CR0_PG_MASK (1u << 31)

typedef uint32_t stage7a_pde_t;
typedef uint32_t stage7a_pte_t;

typedef struct __attribute__((aligned(STAGE7A_PAGING_PAGE_SIZE))) page_table {
    stage7a_pte_t entries[STAGE7A_PAGING_ENTRIES_PER_TABLE];
} page_table_t;

typedef struct __attribute__((aligned(STAGE7A_PAGING_PAGE_SIZE))) page_directory {
    stage7a_pde_t entries[STAGE7A_PAGING_ENTRIES_PER_TABLE];
} page_directory_t;

_Static_assert(sizeof(page_table_t) == STAGE7A_PAGING_PAGE_SIZE, "Stage 7B page table must be exactly 4 KiB");
_Static_assert(sizeof(page_directory_t) == STAGE7A_PAGING_PAGE_SIZE, "Stage 7B page directory must be exactly 4 KiB");
_Static_assert(_Alignof(page_table_t) == STAGE7A_PAGING_PAGE_SIZE, "Stage 7B page table must be 4 KiB aligned");
_Static_assert(_Alignof(page_directory_t) == STAGE7A_PAGING_PAGE_SIZE, "Stage 7B page directory must be 4 KiB aligned");

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

void stage7b_setup_early_identity_paging(void);
const page_directory_t* stage7b_get_early_page_directory(void);
const page_table_t* stage7b_get_early_identity_page_table(void);

void stage7c_load_cr3_with_early_page_directory(void);
void stage7c_set_cr0_paging_enable(void);
uint32_t stage7c_read_cr0(void);
uint32_t stage7c_read_cr3(void);

#endif


#endif /* A6506C01_6A6A_4F36_9170_E6D284EE55A4 */
