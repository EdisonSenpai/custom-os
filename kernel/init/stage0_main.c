#include <stdint.h>

#include "kernel/kobject.h"
#include "mm/kmalloc.h"
#include "mm/kmemory.h"
#include "mm/kheap.h"
#include "mm/paging.h"
#include "mm/vmm.h"
#include "mm/vmm_layout.h"

#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_ATTR 0x0F
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COM1_PORT 0x3F8
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u
#define MULTIBOOT2_TAG_END 0u
#define MULTIBOOT2_TAG_MMAP 6u
#define MULTIBOOT2_MEMORY_AVAILABLE 1u
#define STAGE5B_MAX_REGIONS 128u
#define STAGE5B_POLICY_MIN_ADDR 0x00100000ull
#define STAGE5B_REGION_FLAG_RAW_USABLE 0x01u
#define STAGE5B_REGION_FLAG_POLICY_USABLE 0x02u
#define STAGE5C_MAX_RANGES 128u
#define STAGE5C_FRAME_SHIFT 12u
#define STAGE5C_FRAME_SIZE (1ull << STAGE5C_FRAME_SHIFT)
#define STAGE5D_TEST_ALLOC_COUNT 4u
#define STAGE6C_MAX_PENDING_FREE 64u
#define IDT_ENTRIES 256u
#define IDT_INT_GATE_PRESENT_RING0 0x8Eu
#define IRQ_BASE_VECTOR 0x20u

#define PIC1_COMMAND 0x20u
#define PIC1_DATA 0x21u
#define PIC2_COMMAND 0xA0u
#define PIC2_DATA 0xA1u

#define PIC_ICW1_INIT 0x10u
#define PIC_ICW1_ICW4 0x01u
#define PIC_ICW4_8086 0x01u
#define PIC_EOI 0x20u

#define PIT_CHANNEL0 0x40u
#define PIT_COMMAND 0x43u
#define PIT_INPUT_HZ 1193182u
#define PIT_TARGET_HZ 100u
#define STAGE7D_IDENTITY_MAP_MAX_ADDR 0x003FFFFFu
#define STAGE7D_PROBE_ADDR_LOW_PAGE 0x00001000u
#define STAGE7D_PROBE_ADDR_VGA_TEXT 0x000B8000u
#define STAGE7D_PF_AWARE_ADDR 0x00400000u
#define STAGE8B_TEST_VIRTUAL_PAGE STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START
#define STAGE8B_TEST_PHYSICAL_FRAME 0x00300000u
#define STAGE8D_TEST_ALLOC_COUNT 4u
#define STAGE8D_FAIL_DETAIL 0x000800D0u
#define STAGE9A_FAIL_DETAIL 0x000900A0u
#define STAGE9B_FAIL_DETAIL 0x000900B0u
#define STAGE9C_FAIL_DETAIL 0x000900C0u
#define STAGE9D_FAIL_DETAIL 0x000900D0u
#define STAGE10A_FAIL_DETAIL 0x000A00A0u
#define STAGE10B_FAIL_DETAIL 0x000A00B0u
#define STAGE10C_FAIL_DETAIL 0x000A00C0u
#define STAGE10D_FAIL_DETAIL 0x000A00D0u
#define STAGE10E_FAIL_DETAIL 0x000A00E0u
#define STAGE11A_FAIL_DETAIL 0x000B00A0u
#define STAGE11B_FAIL_DETAIL 0x000B00B0u
#define STAGE11C_FAIL_DETAIL 0x000B00C0u
#define STAGE11D_FAIL_DETAIL 0x000B00D0u

#ifndef STAGE1_FORCE_PANIC
#define STAGE1_FORCE_PANIC 0
#endif

#ifndef STAGE2_FORCE_EXCEPTION
#define STAGE2_FORCE_EXCEPTION 0
#endif

#ifndef STAGE6D_FORCE_REUSE_TEST
#define STAGE6D_FORCE_REUSE_TEST 0
#endif

static volatile uint32_t g_timer_ticks = 0u;

struct multiboot2_info_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_mmap_tag {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed));

struct multiboot2_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

struct stage5b_region {
    uint64_t base_addr;
    uint64_t length;
    uint64_t end_addr;
    uint32_t multiboot_type;
    uint32_t flags;
};

struct stage5b_memory_bookkeeping {
    uint32_t region_count;
    uint32_t dropped_regions;
    uint64_t raw_usable_bytes;
    uint64_t raw_reserved_bytes;
    uint64_t policy_usable_bytes;
    uint64_t highest_usable_end;
    struct stage5b_region regions[STAGE5B_MAX_REGIONS];
};

struct stage5c_frame_range {
    uint64_t start_frame;
    uint64_t frame_count;
};

struct stage5c_frame_bookkeeping {
    uint32_t range_count;
    uint32_t dropped_ranges;
    uint64_t eligible_frames;
    uint64_t highest_eligible_frame;
    uint64_t highest_eligible_addr;
    struct stage5c_frame_range ranges[STAGE5C_MAX_RANGES];
};

/*
 * Stage 6A PMM invariants:
 * - is_initialized is 0 or 1.
 * - range_index is in [0, g_stage5c_frames.range_count].
 * - if range_index < range_count, next_frame_index is at or after that range start.
 * - remaining_frame_count never increases after seeding.
 * - allocation order is deterministic by range order then frame index.
 */
struct stage6a_pmm_state {
    uint32_t is_initialized;
    uint32_t range_index;
    uint64_t next_frame_index;
    uint64_t remaining_frame_count;
    uint64_t allocation_count;
};

struct stage5_parse_totals {
    uint32_t entry_count;
    uint32_t usable_regions;
    uint32_t reserved_regions;
    uint64_t raw_usable_bytes;
    uint64_t raw_reserved_bytes;
};

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry g_idt[IDT_ENTRIES];
static struct idtr g_idtr;
static struct stage5b_memory_bookkeeping g_stage5b_memory;
static struct stage5c_frame_bookkeeping g_stage5c_frames;
static struct stage6a_pmm_state g_stage6a_pmm;
static uint64_t g_stage6c_pending_free_frames[STAGE6C_MAX_PENDING_FREE];
static uint64_t g_stage6c_pending_free_count;
static uint64_t g_stage5c_kernel_reserved_start;
static uint64_t g_stage5c_kernel_reserved_end;

__attribute__((noreturn)) static void panic(const char* reason, uint32_t detail);
static int stage6d_pmm_try_allocate_reused_frame(uint64_t* out_phys_addr);
static void stage7a_run_paging_groundwork_self_check(void);
static void stage7b_run_static_paging_setup_self_check(void);
static void stage7c_run_paging_activation_self_check(void);
static void stage7d_run_identity_validation_self_check(void);
static void stage8a_run_vmm_layout_policy_self_check(void);
static void stage8b_run_vmm_mapping_interface_self_check(void);
static void stage8c_run_kheap_bootstrap_self_check(void);
static void stage8d_run_kheap_validation_self_check(void);
static void stage9a_run_kheap_free_groundwork_self_check(void);
static void stage9b_run_kheap_reuse_activation_self_check(void);
static void stage9c_run_kheap_fragmentation_reuse_self_check(void);
static void stage9d_run_kheap_lifecycle_validation_self_check(void);
static void stage10a_run_kmalloc_interface_self_check(void);
static void stage10b_run_kmalloc_allocation_semantics_self_check(void);
static void stage10c_run_kmemory_primitives_self_check(void);
static void stage10d_run_kmalloc_lifecycle_hardening_self_check(void);
static void stage10e_run_allocation_api_validation_suite_self_check(void);
static void stage11a_run_kobject_groundwork_self_check(void);
static void stage11b_run_kobject_linked_structure_self_check(void);
static void stage11c_run_kobject_traversal_self_check(void);
static void stage11d_run_kobject_removal_self_check(void);

extern uint8_t __kernel_phys_start;
extern uint8_t __kernel_phys_end;

extern void isr_stub_divide_error(void);
extern void isr_stub_breakpoint(void);
extern void isr_stub_invalid_opcode(void);
extern void isr_stub_general_protection(void);
extern void isr_stub_page_fault(void);
extern void irq_stub_timer(void);
extern void irq_stub_keyboard(void);

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait(void)
{
    outb(0x80u, 0u);
}

static void serial_init(void)
{
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}

static int serial_tx_ready(void)
{
    return (inb(COM1_PORT + 5) & 0x20) != 0;
}

static void serial_write_char(char c)
{
    while (!serial_tx_ready()) {
    }

    outb(COM1_PORT, (uint8_t)c);
}

static void serial_write_text(const char* text)
{
    uint16_t i = 0;

    while (text[i] != '\0') {
        if (text[i] == '\n') {
            serial_write_char('\r');
        }

        serial_write_char(text[i]);
        i++;
    }
}

static void clear_screen(void)
{
    volatile uint16_t* vga = VGA_TEXT_BUFFER;
    uint16_t i = 0;
    const uint16_t blank = ((uint16_t)VGA_ATTR << 8) | (uint8_t)' ';

    while (i < (VGA_WIDTH * VGA_HEIGHT)) {
        vga[i] = blank;
        i++;
    }
}

static void write_text_at(const char* text, uint16_t row, uint16_t col_start)
{
    uint16_t col = col_start;
    volatile uint16_t* vga = VGA_TEXT_BUFFER + (row * VGA_WIDTH);

    while (*text != '\0' && col < VGA_WIDTH) {
        vga[col] = ((uint16_t)VGA_ATTR << 8) | (uint8_t)*text;
        text++;
        col++;
    }
}

static void write_text(const char* text, uint16_t row)
{
    write_text_at(text, row, 0);
}

static void format_hex32(uint32_t value, char out[11])
{
    static const char hex[] = "0123456789ABCDEF";
    uint32_t i = 0;

    out[0] = '0';
    out[1] = 'x';

    while (i < 8) {
        const uint32_t shift = (7u - i) * 4u;
        out[2u + i] = hex[(value >> shift) & 0xFu];
        i++;
    }

    out[10] = '\0';
}

static void format_hex64(uint64_t value, char out[19])
{
    static const char hex[] = "0123456789ABCDEF";
    uint32_t i = 0;

    out[0] = '0';
    out[1] = 'x';

    while (i < 16u) {
        const uint32_t shift = (15u - i) * 4u;
        out[2u + i] = hex[(value >> shift) & 0xFu];
        i++;
    }

    out[18] = '\0';
}

static void write_hex_at_row(const char* label, uint16_t row, uint16_t value_col, uint32_t value)
{
    char value_hex[11];

    format_hex32(value, value_hex);
    write_text(label, row);
    write_text_at(value_hex, row, value_col);
}

static void serial_write_label_hex(const char* label, uint32_t value)
{
    char value_hex[11];

    format_hex32(value, value_hex);
    serial_write_text(label);
    serial_write_text(value_hex);
    serial_write_text("\n");
}

static void serial_write_label_hex64(const char* label, uint64_t value)
{
    char value_hex[19];

    format_hex64(value, value_hex);
    serial_write_text(label);
    serial_write_text(value_hex);
    serial_write_text("\n");
}

static uint32_t align_up_8(uint32_t value)
{
    return (value + 7u) & ~7u;
}

static uint64_t add_clamp_u64(uint64_t accumulator, uint64_t increment)
{
    const uint64_t sum = accumulator + increment;

    if (sum < accumulator) {
        return UINT64_MAX;
    }

    return sum;
}

static int checked_add_u64(uint64_t a, uint64_t b, uint64_t* out)
{
    *out = a + b;
    return *out < a;
}

static void stage5b_reset_bookkeeping(void)
{
    g_stage5b_memory.region_count = 0u;
    g_stage5b_memory.dropped_regions = 0u;
    g_stage5b_memory.raw_usable_bytes = 0u;
    g_stage5b_memory.raw_reserved_bytes = 0u;
    g_stage5b_memory.policy_usable_bytes = 0u;
    g_stage5b_memory.highest_usable_end = 0u;
}

static void stage5b_record_region(uint64_t base, uint64_t length, uint64_t end, uint32_t type, uint32_t flags)
{
    const uint32_t index = g_stage5b_memory.region_count;

    /* Region storage is capped for Stage 5B reviewability; byte totals are tracked separately. */
    if (index >= STAGE5B_MAX_REGIONS) {
        g_stage5b_memory.dropped_regions++;
        return;
    }

    g_stage5b_memory.regions[index].base_addr = base;
    g_stage5b_memory.regions[index].length = length;
    g_stage5b_memory.regions[index].end_addr = end;
    g_stage5b_memory.regions[index].multiboot_type = type;
    g_stage5b_memory.regions[index].flags = flags;
    g_stage5b_memory.region_count = index + 1u;
}

static uint64_t stage5b_policy_usable_length(uint64_t base, uint64_t length, uint32_t type)
{
    uint64_t end = 0u;

    if (type != MULTIBOOT2_MEMORY_AVAILABLE || length == 0u) {
        return 0u;
    }

    if (checked_add_u64(base, length, &end) != 0) {
        return 0u;
    }

    if (end <= STAGE5B_POLICY_MIN_ADDR) {
        return 0u;
    }

    if (base >= STAGE5B_POLICY_MIN_ADDR) {
        return length;
    }

    return end - STAGE5B_POLICY_MIN_ADDR;
}

static void stage5_parse_totals_reset(struct stage5_parse_totals* totals)
{
    totals->entry_count = 0u;
    totals->usable_regions = 0u;
    totals->reserved_regions = 0u;
    totals->raw_usable_bytes = 0u;
    totals->raw_reserved_bytes = 0u;
}

static void stage5b_account_entry(const struct multiboot2_mmap_entry* entry)
{
    uint32_t region_flags = 0u;
    uint64_t region_end = entry->base_addr;
    uint64_t policy_usable_length = 0u;

    if (checked_add_u64(entry->base_addr, entry->length, &region_end) != 0) {
        region_end = UINT64_MAX;
    }

    if (entry->type == MULTIBOOT2_MEMORY_AVAILABLE) {
        region_flags |= STAGE5B_REGION_FLAG_RAW_USABLE;
    }

    policy_usable_length = stage5b_policy_usable_length(entry->base_addr, entry->length, entry->type);

    if (policy_usable_length > 0u) {
        region_flags |= STAGE5B_REGION_FLAG_POLICY_USABLE;
        g_stage5b_memory.policy_usable_bytes = add_clamp_u64(
            g_stage5b_memory.policy_usable_bytes,
            policy_usable_length);

        if (region_end > g_stage5b_memory.highest_usable_end) {
            g_stage5b_memory.highest_usable_end = region_end;
        }
    }

    stage5b_record_region(entry->base_addr, entry->length, region_end, entry->type, region_flags);
}

static void stage5b_emit_summary(const struct stage5_parse_totals* totals)
{
    const uint64_t policy_unusable_bytes =
        (g_stage5b_memory.raw_usable_bytes >= g_stage5b_memory.policy_usable_bytes)
            ? (g_stage5b_memory.raw_usable_bytes - g_stage5b_memory.policy_usable_bytes)
            : 0u;

    serial_write_label_hex("custom-os Stage 5A mmap entries: ", totals->entry_count);
    serial_write_label_hex("custom-os Stage 5A usable regions: ", totals->usable_regions);
    serial_write_label_hex("custom-os Stage 5A reserved regions: ", totals->reserved_regions);
    serial_write_label_hex64("custom-os Stage 5A usable bytes: ", totals->raw_usable_bytes);

    serial_write_label_hex("custom-os Stage 5B regions stored: ", g_stage5b_memory.region_count);
    serial_write_label_hex("custom-os Stage 5B regions dropped: ", g_stage5b_memory.dropped_regions);
    serial_write_label_hex64("custom-os Stage 5B raw usable bytes: ", g_stage5b_memory.raw_usable_bytes);
    serial_write_label_hex64("custom-os Stage 5B raw reserved bytes: ", g_stage5b_memory.raw_reserved_bytes);
    serial_write_label_hex64("custom-os Stage 5B policy usable bytes: ", g_stage5b_memory.policy_usable_bytes);
    serial_write_label_hex64("custom-os Stage 5B policy unavailable bytes: ", policy_unusable_bytes);
    serial_write_label_hex64("custom-os Stage 5B highest usable end: ", g_stage5b_memory.highest_usable_end);
}

static void stage5c_reset_bookkeeping(void)
{
    g_stage5c_frames.range_count = 0u;
    g_stage5c_frames.dropped_ranges = 0u;
    g_stage5c_frames.eligible_frames = 0u;
    g_stage5c_frames.highest_eligible_frame = 0u;
    g_stage5c_frames.highest_eligible_addr = 0u;
    g_stage5c_kernel_reserved_start = 0u;
    g_stage5c_kernel_reserved_end = 0u;
}

static void stage5c_record_range(uint64_t start_frame, uint64_t frame_count)
{
    const uint32_t index = g_stage5c_frames.range_count;

    if (index >= STAGE5C_MAX_RANGES) {
        g_stage5c_frames.dropped_ranges++;
        return;
    }

    g_stage5c_frames.ranges[index].start_frame = start_frame;
    g_stage5c_frames.ranges[index].frame_count = frame_count;
    g_stage5c_frames.range_count = index + 1u;
}

static uint64_t stage5c_align_up_4k(uint64_t value, uint32_t* overflow)
{
    uint64_t rounded = 0u;

    if (checked_add_u64(value, STAGE5C_FRAME_SIZE - 1u, &rounded) != 0) {
        *overflow = 1u;
        return UINT64_MAX;
    }

    *overflow = 0u;
    return rounded & ~(STAGE5C_FRAME_SIZE - 1u);
}

static void stage5c_emit_summary(void)
{
    serial_write_label_hex("custom-os Stage 5C range count: ", g_stage5c_frames.range_count);
    serial_write_label_hex64("custom-os Stage 5C eligible frames: ", g_stage5c_frames.eligible_frames);
    serial_write_label_hex64("custom-os Stage 5C highest eligible frame: ", g_stage5c_frames.highest_eligible_frame);
    serial_write_label_hex64("custom-os Stage 5C highest eligible address: ", g_stage5c_frames.highest_eligible_addr);
    serial_write_label_hex64("custom-os Stage 5C reserved kernel start: ", g_stage5c_kernel_reserved_start);
    serial_write_label_hex64("custom-os Stage 5C reserved kernel end: ", g_stage5c_kernel_reserved_end);
}

static void stage5c_record_eligible_span(uint64_t start_addr, uint64_t end_addr)
{
    uint64_t frame_count = 0u;
    uint64_t start_frame = 0u;
    uint64_t highest_frame_addr = 0u;
    uint64_t highest_frame = 0u;
    uint32_t overflow = 0u;

    if (end_addr <= start_addr) {
        return;
    }

    start_addr = stage5c_align_up_4k(start_addr, &overflow);
    if (overflow != 0u) {
        return;
    }

    end_addr &= ~(STAGE5C_FRAME_SIZE - 1u);

    if (end_addr <= start_addr) {
        return;
    }

    frame_count = (end_addr - start_addr) >> STAGE5C_FRAME_SHIFT;
    start_frame = start_addr >> STAGE5C_FRAME_SHIFT;
    highest_frame_addr = end_addr - STAGE5C_FRAME_SIZE;
    highest_frame = highest_frame_addr >> STAGE5C_FRAME_SHIFT;

    stage5c_record_range(start_frame, frame_count);
    g_stage5c_frames.eligible_frames = add_clamp_u64(g_stage5c_frames.eligible_frames, frame_count);

    if (highest_frame > g_stage5c_frames.highest_eligible_frame) {
        g_stage5c_frames.highest_eligible_frame = highest_frame;
        g_stage5c_frames.highest_eligible_addr = highest_frame_addr;
    }
}

static void stage5c_prepare_frame_bookkeeping(void)
{
    const uint64_t kernel_phys_start_raw = (uint64_t)(uintptr_t)&__kernel_phys_start;
    const uint64_t kernel_phys_end_raw = (uint64_t)(uintptr_t)&__kernel_phys_end;
    uint64_t kernel_reserved_start = 0u;
    uint64_t kernel_reserved_end = 0u;
    uint32_t kernel_align_overflow = 0u;
    uint32_t i = 0u;

    stage5c_reset_bookkeeping();

    kernel_reserved_start = kernel_phys_start_raw & ~(STAGE5C_FRAME_SIZE - 1u);
    kernel_reserved_end = stage5c_align_up_4k(kernel_phys_end_raw, &kernel_align_overflow);

    if (kernel_align_overflow != 0u || kernel_reserved_end <= kernel_reserved_start) {
        panic("invalid kernel reservation bounds", 0x000500C0u);
    }

    g_stage5c_kernel_reserved_start = kernel_reserved_start;
    g_stage5c_kernel_reserved_end = kernel_reserved_end;

    while (i < g_stage5b_memory.region_count) {
        const struct stage5b_region* region = &g_stage5b_memory.regions[i];
        uint64_t start_addr = region->base_addr;
        uint64_t end_addr = region->end_addr;

        if ((region->flags & STAGE5B_REGION_FLAG_POLICY_USABLE) == 0u) {
            i++;
            continue;
        }

        if (region->multiboot_type != MULTIBOOT2_MEMORY_AVAILABLE) {
            i++;
            continue;
        }

        if (start_addr < STAGE5B_POLICY_MIN_ADDR) {
            start_addr = STAGE5B_POLICY_MIN_ADDR;
        }

        if (end_addr <= start_addr) {
            i++;
            continue;
        }

        if (end_addr <= kernel_reserved_start || start_addr >= kernel_reserved_end) {
            stage5c_record_eligible_span(start_addr, end_addr);
            i++;
            continue;
        }

        if (start_addr < kernel_reserved_start) {
            stage5c_record_eligible_span(start_addr, kernel_reserved_start);
        }

        if (end_addr > kernel_reserved_end) {
            stage5c_record_eligible_span(kernel_reserved_end, end_addr);
        }

        i++;
    }

    stage5c_emit_summary();
}

static void stage6a_pmm_reset_state(void)
{
    g_stage6a_pmm.is_initialized = 0u;
    g_stage6a_pmm.range_index = 0u;
    g_stage6a_pmm.next_frame_index = 0u;
    g_stage6a_pmm.remaining_frame_count = 0u;
    g_stage6a_pmm.allocation_count = 0u;
}

static void stage6a_pmm_advance_to_next_eligible(void)
{
    while (g_stage6a_pmm.range_index < g_stage5c_frames.range_count) {
        const struct stage5c_frame_range* range = &g_stage5c_frames.ranges[g_stage6a_pmm.range_index];
        uint64_t range_end_frame = 0u;

        if (range->frame_count == 0u) {
            g_stage6a_pmm.range_index++;
            continue;
        }

        if (checked_add_u64(range->start_frame, range->frame_count, &range_end_frame) != 0) {
            range_end_frame = UINT64_MAX;
        }

        if (g_stage6a_pmm.next_frame_index < range->start_frame) {
            g_stage6a_pmm.next_frame_index = range->start_frame;
        }

        if (g_stage6a_pmm.next_frame_index < range_end_frame) {
            break;
        }

        g_stage6a_pmm.range_index++;
    }
}

static void stage6a_pmm_seed_from_stage5c(void)
{
    stage6a_pmm_reset_state();

    g_stage6a_pmm.remaining_frame_count = g_stage5c_frames.eligible_frames;

    if (g_stage5c_frames.range_count == 0u || g_stage5c_frames.eligible_frames == 0u) {
        g_stage6a_pmm.is_initialized = 1u;
        return;
    }

    g_stage6a_pmm.next_frame_index = g_stage5c_frames.ranges[0].start_frame;
    stage6a_pmm_advance_to_next_eligible();
    g_stage6a_pmm.is_initialized = 1u;
}

static int stage6a_pmm_try_allocate_frame(uint64_t* out_phys_addr)
{
    uint64_t frame = 0u;

    if (out_phys_addr == (uint64_t*)0) {
        return 0;
    }

    if (g_stage6a_pmm.is_initialized == 0u) {
        stage6a_pmm_seed_from_stage5c();
    }

    if (g_stage6a_pmm.remaining_frame_count == 0u) {
        return 0;
    }

    stage6a_pmm_advance_to_next_eligible();

    if (g_stage6a_pmm.range_index >= g_stage5c_frames.range_count) {
        return 0;
    }

    frame = g_stage6a_pmm.next_frame_index;
    g_stage6a_pmm.next_frame_index = frame + 1u;
    g_stage6a_pmm.remaining_frame_count--;
    g_stage6a_pmm.allocation_count = add_clamp_u64(g_stage6a_pmm.allocation_count, 1u);

    if (frame > (UINT64_MAX >> STAGE5C_FRAME_SHIFT)) {
        return 0;
    }

    *out_phys_addr = frame << STAGE5C_FRAME_SHIFT;
    return 1;
}

int stage6b_pmm_alloc_frame(uint64_t* out_phys_addr)
{
    if (stage6d_pmm_try_allocate_reused_frame(out_phys_addr) != 0) {
        return 1;
    }

    return stage6a_pmm_try_allocate_frame(out_phys_addr);
}

uint64_t stage6b_pmm_get_remaining_frames(void)
{
    return g_stage6a_pmm.remaining_frame_count;
}

static int stage6d_pmm_pop_pending_free_fifo(uint64_t* out_frame)
{
    uint64_t i = 0u;

    if (out_frame == (uint64_t*)0) {
        return 0;
    }

    if (g_stage6c_pending_free_count == 0u) {
        return 0;
    }

    *out_frame = g_stage6c_pending_free_frames[0];

    while ((i + 1u) < g_stage6c_pending_free_count) {
        g_stage6c_pending_free_frames[i] = g_stage6c_pending_free_frames[i + 1u];
        i++;
    }

    g_stage6c_pending_free_count--;
    return 1;
}

static int stage6d_pmm_try_allocate_reused_frame(uint64_t* out_phys_addr)
{
    uint64_t frame = 0u;

    if (out_phys_addr == (uint64_t*)0) {
        return 0;
    }

    if (g_stage6a_pmm.remaining_frame_count == 0u) {
        return 0;
    }

    if (stage6d_pmm_pop_pending_free_fifo(&frame) == 0) {
        return 0;
    }

    if (frame > (UINT64_MAX >> STAGE5C_FRAME_SHIFT)) {
        return 0;
    }

    g_stage6a_pmm.remaining_frame_count--;
    g_stage6a_pmm.allocation_count = add_clamp_u64(g_stage6a_pmm.allocation_count, 1u);
    *out_phys_addr = frame << STAGE5C_FRAME_SHIFT;
    return 1;
}

static int stage6d_pmm_find_frame_range(uint64_t frame, uint32_t* out_range_index)
{
    uint32_t i = 0u;

    while (i < g_stage5c_frames.range_count) {
        const struct stage5c_frame_range* range = &g_stage5c_frames.ranges[i];
        uint64_t range_end = 0u;

        if (range->frame_count == 0u) {
            i++;
            continue;
        }

        if (checked_add_u64(range->start_frame, range->frame_count, &range_end) != 0) {
            range_end = UINT64_MAX;
        }

        if (frame >= range->start_frame && frame < range_end) {
            if (out_range_index != (uint32_t*)0) {
                *out_range_index = i;
            }

            return 1;
        }

        i++;
    }

    return 0;
}

static int stage6c_is_eligible_frame(uint64_t frame)
{
    return stage6d_pmm_find_frame_range(frame, (uint32_t*)0);
}

static int stage6d_pmm_was_frame_issued(uint64_t frame)
{
    uint32_t frame_range_index = 0u;

    if (g_stage6a_pmm.is_initialized == 0u) {
        return 0;
    }

    if (stage6d_pmm_find_frame_range(frame, &frame_range_index) == 0) {
        return 0;
    }

    if (g_stage6a_pmm.range_index >= g_stage5c_frames.range_count) {
        return 1;
    }

    if (frame_range_index < g_stage6a_pmm.range_index) {
        return 1;
    }

    if (frame_range_index > g_stage6a_pmm.range_index) {
        return 0;
    }

    return frame < g_stage6a_pmm.next_frame_index;
}

int stage6c_pmm_free_frame(uint64_t phys_addr)
{
    uint64_t frame = 0u;
    uint64_t i = 0u;

    if ((phys_addr & (STAGE5C_FRAME_SIZE - 1u)) != 0u) {
        return 0;
    }

    if (phys_addr < STAGE5B_POLICY_MIN_ADDR) {
        return 0;
    }

    frame = phys_addr >> STAGE5C_FRAME_SHIFT;

    if (stage6c_is_eligible_frame(frame) == 0) {
        return 0;
    }

    if (stage6d_pmm_was_frame_issued(frame) == 0) {
        return 0;
    }

    while (i < g_stage6c_pending_free_count) {
        if (g_stage6c_pending_free_frames[i] == frame) {
            return 0;
        }

        i++;
    }

    if (g_stage6c_pending_free_count >= STAGE6C_MAX_PENDING_FREE) {
        return 0;
    }

    g_stage6c_pending_free_frames[g_stage6c_pending_free_count] = frame;
    g_stage6c_pending_free_count++;
    g_stage6a_pmm.remaining_frame_count = add_clamp_u64(g_stage6a_pmm.remaining_frame_count, 1u);
    return 1;
}

int stage6c_pmm_get_pending_free_frames(uint64_t* out_count)
{
    if (out_count == (uint64_t*)0) {
        return 0;
    }

    *out_count = g_stage6c_pending_free_count;
    return 1;
}

#if STAGE6D_FORCE_REUSE_TEST
static void stage6d_emit_test_result(const char* check_name, int passed)
{
    serial_write_text("custom-os Stage 6D test ");
    serial_write_text(check_name);
    serial_write_text(": ");

    if (passed != 0) {
        serial_write_text("PASS\n");
    } else {
        serial_write_text("FAIL\n");
    }
}

static void stage6d_run_reuse_self_test(void)
{
    uint64_t alloc_a = 0u;
    uint64_t alloc_b = 0u;
    uint64_t reuse_a = 0u;
    uint64_t reuse_b = 0u;
    uint64_t never_issued_addr = 0u;
    uint64_t pending_count = 0u;
    int check = 0;
    int all_passed = 1;

    serial_write_text("custom-os Stage 6D test: start\n");

    check = (stage6b_pmm_alloc_frame(&alloc_a) != 0)
        && (stage6b_pmm_alloc_frame(&alloc_b) != 0)
        && (alloc_a != alloc_b);
    stage6d_emit_test_result("alloc A/B", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6c_pmm_free_frame(alloc_a) != 0);
    stage6d_emit_test_result("free A accept", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6c_pmm_free_frame(alloc_b) != 0);
    stage6d_emit_test_result("free B accept", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6c_pmm_free_frame(alloc_a) == 0);
    stage6d_emit_test_result("duplicate free reject", check);
    if (check == 0) {
        all_passed = 0;
    }

    stage6a_pmm_advance_to_next_eligible();
    if (g_stage6a_pmm.range_index < g_stage5c_frames.range_count
        && g_stage6a_pmm.next_frame_index <= (UINT64_MAX >> STAGE5C_FRAME_SHIFT)) {
        never_issued_addr = g_stage6a_pmm.next_frame_index << STAGE5C_FRAME_SHIFT;
        check = (stage6c_pmm_free_frame(never_issued_addr) == 0);
    } else {
        check = 0;
    }

    stage6d_emit_test_result("never-issued reject", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6c_pmm_get_pending_free_frames(&pending_count) != 0)
        && (pending_count == 2u);
    stage6d_emit_test_result("pending count after free", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6b_pmm_alloc_frame(&reuse_a) != 0)
        && (stage6b_pmm_alloc_frame(&reuse_b) != 0)
        && (reuse_a == alloc_a)
        && (reuse_b == alloc_b);
    stage6d_emit_test_result("fifo reuse order", check);
    if (check == 0) {
        all_passed = 0;
    }

    check = (stage6c_pmm_get_pending_free_frames(&pending_count) != 0)
        && (pending_count == 0u);
    stage6d_emit_test_result("pending drained", check);
    if (check == 0) {
        all_passed = 0;
    }

    stage6d_emit_test_result("overall", all_passed);
}
#endif

static void stage5d_run_boot_allocation_test(void)
{
    uint32_t granted = 0u;
    uint32_t i = 0u;

    stage6a_pmm_seed_from_stage5c();

    serial_write_text("custom-os Stage 5D: deterministic frame allocation test\n");
    serial_write_label_hex("custom-os Stage 5D alloc request count: ", STAGE5D_TEST_ALLOC_COUNT);

    while (i < STAGE5D_TEST_ALLOC_COUNT) {
        uint64_t phys_addr = 0u;

        if (stage6b_pmm_alloc_frame(&phys_addr) == 0) {
            break;
        }

        serial_write_label_hex("custom-os Stage 5D alloc index: ", i);
        serial_write_label_hex64("custom-os Stage 5D alloc addr : ", phys_addr);
        granted++;
        i++;
    }

    serial_write_label_hex("custom-os Stage 5D alloc granted: ", granted);
    serial_write_label_hex64("custom-os Stage 5D remaining eligible frames: ", stage6b_pmm_get_remaining_frames());
}

static void stage7a_run_paging_groundwork_self_check(void)
{
    const uint32_t sample_vaddr = 0x12345000u;
    const uint32_t sample_unaligned_addr = 0x12345ABCu;
    const uint32_t expected_pd_index = 0x48u;
    const uint32_t expected_pt_index = 0x345u;
    const uint32_t expected_page_offset = 0x0u;
    const uint32_t expected_identity_addr = 0x12345000u;
    const uint32_t expected_entry = 0x12345003u;
    const uint32_t pd_index = stage7a_paging_pd_index(sample_vaddr);
    const uint32_t pt_index = stage7a_paging_pt_index(sample_vaddr);
    const uint32_t page_offset = stage7a_paging_page_offset(sample_vaddr);
    const uint32_t identity_addr = stage7a_paging_identity_map_addr(sample_unaligned_addr);
    const uint32_t entry = stage7a_paging_make_entry(
        sample_unaligned_addr,
        STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE);
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 7A: paging groundwork self-check\n");
    serial_write_label_hex("custom-os Stage 7A sample vaddr: ", sample_vaddr);
    serial_write_label_hex("custom-os Stage 7A pd index    : ", pd_index);
    serial_write_label_hex("custom-os Stage 7A pt index    : ", pt_index);
    serial_write_label_hex("custom-os Stage 7A page offset : ", page_offset);
    serial_write_label_hex("custom-os Stage 7A identity addr: ", identity_addr);
    serial_write_label_hex("custom-os Stage 7A entry source: ", sample_unaligned_addr);
    serial_write_label_hex("custom-os Stage 7A entry value : ", entry);

    if (pd_index != expected_pd_index) {
        passed = 0u;
    }

    if (pt_index != expected_pt_index) {
        passed = 0u;
    }

    if (page_offset != expected_page_offset) {
        passed = 0u;
    }

    if (identity_addr != expected_identity_addr) {
        passed = 0u;
    }

    if (entry != expected_entry) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 7A self-check: PASS\n");
    } else {
        serial_write_text("custom-os Stage 7A self-check: FAIL\n");
    }
}

static void stage7b_run_static_paging_setup_self_check(void)
{
    const uint32_t expected_structure_size = STAGE7A_PAGING_PAGE_SIZE;
    const uint32_t expected_first_pte = 0x00000003u;
    const uint32_t expected_last_pte = 0x003FF003u;
    const uint32_t expected_pde0 = stage7a_paging_make_entry(
        (uint32_t)(uintptr_t)stage7b_get_early_identity_page_table(),
        STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE);
    const uint32_t page_table_size = (uint32_t)sizeof(page_table_t);
    const uint32_t page_directory_size = (uint32_t)sizeof(page_directory_t);
    const page_table_t* identity_page_table = 0;
    const page_directory_t* page_directory = 0;
    uint32_t first_pte = 0u;
    uint32_t last_pte = 0u;
    uint32_t pde0 = 0u;
    uint32_t passed = 1u;

    stage7b_setup_early_identity_paging();

    identity_page_table = stage7b_get_early_identity_page_table();
    page_directory = stage7b_get_early_page_directory();
    first_pte = identity_page_table->entries[0];
    last_pte = identity_page_table->entries[STAGE7A_PAGING_ENTRIES_PER_TABLE - 1u];
    pde0 = page_directory->entries[0];

    serial_write_text("custom-os Stage 7B: static paging setup self-check\n");
    serial_write_label_hex("custom-os Stage 7B page_table_t size : ", page_table_size);
    serial_write_label_hex("custom-os Stage 7B page_directory_t size: ", page_directory_size);
    serial_write_label_hex("custom-os Stage 7B first PTE         : ", first_pte);
    serial_write_label_hex("custom-os Stage 7B last PTE          : ", last_pte);
    serial_write_label_hex("custom-os Stage 7B PDE[0]            : ", pde0);

    if (page_table_size != expected_structure_size) {
        passed = 0u;
    }

    if (page_directory_size != expected_structure_size) {
        passed = 0u;
    }

    if (first_pte != expected_first_pte) {
        passed = 0u;
    }

    if (last_pte != expected_last_pte) {
        passed = 0u;
    }

    if (pde0 != expected_pde0) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 7B self-check: PASS\n");
    } else {
        serial_write_text("custom-os Stage 7B self-check: FAIL\n");
    }
}

static void stage7c_run_paging_activation_self_check(void)
{
    const uint32_t expected_cr3 =
        stage7a_paging_frame_addr((uint32_t)(uintptr_t)stage7b_get_early_page_directory());
    uint32_t observed_cr3 = 0u;
    uint32_t observed_cr0 = 0u;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 7C: paging activation begin\n");
    serial_write_label_hex("custom-os Stage 7C expected CR3   : ", expected_cr3);

    stage7c_load_cr3_with_early_page_directory();
    stage7c_set_cr0_paging_enable();

    observed_cr3 = stage7c_read_cr3();
    observed_cr0 = stage7c_read_cr0();

    serial_write_label_hex("custom-os Stage 7C observed CR3   : ", observed_cr3);
    serial_write_label_hex("custom-os Stage 7C observed CR0   : ", observed_cr0);

    if (stage7a_paging_frame_addr(observed_cr3) != expected_cr3) {
        passed = 0u;
    }

    if ((observed_cr0 & STAGE7C_CR0_PG_MASK) == 0u) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 7C activation: PASS\n");
    } else {
        serial_write_text("custom-os Stage 7C activation: FAIL\n");
        panic("Stage 7C paging activation failed", observed_cr0);
    }
}

static uint32_t stage7d_probe_read32(uint32_t addr)
{
    const volatile uint32_t* probe_ptr = (const volatile uint32_t*)(uintptr_t)addr;

    return *probe_ptr;
}

static void stage7d_run_identity_validation_self_check(void)
{
    const uint32_t expected_cr3 =
        stage7a_paging_frame_addr((uint32_t)(uintptr_t)stage7b_get_early_page_directory());
    const page_directory_t* page_directory = stage7b_get_early_page_directory();
    const page_table_t* identity_page_table = stage7b_get_early_identity_page_table();
    const uint32_t probe_low_pt_index = stage7a_paging_pt_index(STAGE7D_PROBE_ADDR_LOW_PAGE);
    const uint32_t probe_vga_pt_index = stage7a_paging_pt_index(STAGE7D_PROBE_ADDR_VGA_TEXT);
    const uint32_t probe_low_expected_pte = stage7a_paging_make_entry(
        STAGE7D_PROBE_ADDR_LOW_PAGE,
        STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE);
    const uint32_t probe_vga_expected_pte = stage7a_paging_make_entry(
        STAGE7D_PROBE_ADDR_VGA_TEXT,
        STAGE7A_PAGING_FLAG_PRESENT | STAGE7A_PAGING_FLAG_WRITABLE);
    uint32_t observed_cr3 = 0u;
    uint32_t observed_cr0 = 0u;
    uint32_t probe_low_pte = 0u;
    uint32_t probe_vga_pte = 0u;
    uint32_t probe_low_read = 0u;
    uint32_t probe_vga_read = 0u;
    uint32_t page_fault_awareness_ok = 1u;
    uint32_t passed = 1u;

    observed_cr3 = stage7c_read_cr3();
    observed_cr0 = stage7c_read_cr0();
    probe_low_pte = identity_page_table->entries[probe_low_pt_index];
    probe_vga_pte = identity_page_table->entries[probe_vga_pt_index];

    serial_write_text("custom-os Stage 7D validation begin\n");
    serial_write_label_hex("custom-os Stage 7D observed CR3 under active paging: ", observed_cr3);
    serial_write_label_hex("custom-os Stage 7D observed CR0 under active paging: ", observed_cr0);

    probe_low_read = stage7d_probe_read32(STAGE7D_PROBE_ADDR_LOW_PAGE);
    probe_vga_read = stage7d_probe_read32(STAGE7D_PROBE_ADDR_VGA_TEXT);

    serial_write_text("custom-os Stage 7D identity-map probe results\n");
    serial_write_label_hex("custom-os Stage 7D probe low addr : ", STAGE7D_PROBE_ADDR_LOW_PAGE);
    serial_write_label_hex("custom-os Stage 7D probe low pte  : ", probe_low_pte);
    serial_write_label_hex("custom-os Stage 7D probe low read : ", probe_low_read);
    serial_write_label_hex("custom-os Stage 7D probe vga addr : ", STAGE7D_PROBE_ADDR_VGA_TEXT);
    serial_write_label_hex("custom-os Stage 7D probe vga pte  : ", probe_vga_pte);
    serial_write_label_hex("custom-os Stage 7D probe vga read : ", probe_vga_read);

    if ((observed_cr0 & STAGE7C_CR0_PG_MASK) == 0u) {
        passed = 0u;
    }

    if (stage7a_paging_frame_addr(observed_cr3) != expected_cr3) {
        passed = 0u;
    }

    if (stage7a_paging_pd_index(STAGE7D_PROBE_ADDR_LOW_PAGE) != 0u
        || stage7a_paging_pd_index(STAGE7D_PROBE_ADDR_VGA_TEXT) != 0u) {
        passed = 0u;
    }

    if (page_directory->entries[0] == 0u) {
        passed = 0u;
    }

    if (probe_low_pte != probe_low_expected_pte) {
        passed = 0u;
    }

    if (probe_vga_pte != probe_vga_expected_pte) {
        passed = 0u;
    }

    if (stage7a_paging_pd_index(STAGE7D_PF_AWARE_ADDR) != 1u
        || STAGE7D_PF_AWARE_ADDR <= STAGE7D_IDENTITY_MAP_MAX_ADDR
        || page_directory->entries[1] != 0u) {
        page_fault_awareness_ok = 0u;
        passed = 0u;
    }

    serial_write_label_hex("custom-os Stage 7D page-fault-aware addr : ", STAGE7D_PF_AWARE_ADDR);
    serial_write_label_hex("custom-os Stage 7D page-fault-aware PDE[1]: ", page_directory->entries[1]);

    if (page_fault_awareness_ok != 0u) {
        serial_write_text("custom-os Stage 7D page-fault-awareness confirmation: PASS (guarded, no forced fault)\n");
    } else {
        serial_write_text("custom-os Stage 7D page-fault-awareness confirmation: FAIL\n");
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 7D validation: PASS\n");
    } else {
        serial_write_text("custom-os Stage 7D validation: FAIL\n");
        panic("Stage 7D paging validation failed", observed_cr0);
    }
}

static void stage8a_run_vmm_layout_policy_self_check(void)
{
    struct stage8a_classification_sample {
        uint32_t addr;
        stage8a_vmm_layout_region_class_t expected_class;
    };

    static const struct stage8a_classification_sample samples[] = {
        {0x00001000u, STAGE8A_VMM_LAYOUT_REGION_ACTIVE_IDENTITY},
        {0x00400000u, STAGE8A_VMM_LAYOUT_REGION_FUTURE_HEAP_RESERVED},
        {0x00800000u, STAGE8A_VMM_LAYOUT_REGION_FUTURE_MAPPING_RESERVED},
        {0x01000000u, STAGE8A_VMM_LAYOUT_REGION_UNCLASSIFIED}
    };

    const uint32_t sample_count = (uint32_t)(sizeof(samples) / sizeof(samples[0]));
    uint32_t i = 0u;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 8A: layout policy begin\n");

    serial_write_text("custom-os Stage 8A active identity-mapped range\n");
    serial_write_label_hex("custom-os Stage 8A active start : ", STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START);
    serial_write_label_hex("custom-os Stage 8A active end   : ", STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END);

    serial_write_text("custom-os Stage 8A future reserved heap range\n");
    serial_write_label_hex("custom-os Stage 8A heap start   : ", STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START);
    serial_write_label_hex("custom-os Stage 8A heap end     : ", STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END);

    serial_write_text("custom-os Stage 8A future reserved mapping range\n");
    serial_write_label_hex("custom-os Stage 8A map start    : ", STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START);
    serial_write_label_hex("custom-os Stage 8A map end      : ", STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END);

    if (stage8a_vmm_layout_validate_policy() == 0) {
        passed = 0u;
    }

    if (stage8a_vmm_layout_is_region_in_active_identity(
            STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START,
            STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_END_EXCLUSIVE - STAGE8A_VMM_LAYOUT_ACTIVE_IDENTITY_START)
        == 0) {
        passed = 0u;
    }

    if (stage8a_vmm_layout_is_region_in_future_heap_reserved(
            STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START,
            STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE - STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START)
        == 0) {
        passed = 0u;
    }

    if (stage8a_vmm_layout_is_region_in_future_mapping_reserved(
            STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START,
            STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_END_EXCLUSIVE - STAGE8A_VMM_LAYOUT_FUTURE_MAPPING_RESERVED_START)
        == 0) {
        passed = 0u;
    }

    if (stage8a_vmm_layout_is_region_in_active_identity(
            STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START,
            STAGE8A_VMM_LAYOUT_PAGE_SIZE_4K)
        != 0) {
        passed = 0u;
    }

    serial_write_text("custom-os Stage 8A address classification examples\n");

    while (i < sample_count) {
        const stage8a_vmm_layout_region_class_t observed_class =
            stage8a_vmm_layout_classify_addr(samples[i].addr);

        serial_write_label_hex("custom-os Stage 8A sample addr  : ", samples[i].addr);
        serial_write_text("custom-os Stage 8A sample class : ");
        serial_write_text(stage8a_vmm_layout_region_class_name(observed_class));
        serial_write_text("\n");

        if (observed_class != samples[i].expected_class) {
            passed = 0u;
        }

        if (samples[i].expected_class == STAGE8A_VMM_LAYOUT_REGION_ACTIVE_IDENTITY
            && stage8a_vmm_layout_is_addr_in_active_identity(samples[i].addr) == 0) {
            passed = 0u;
        }

        if (samples[i].expected_class == STAGE8A_VMM_LAYOUT_REGION_FUTURE_HEAP_RESERVED
            && stage8a_vmm_layout_is_addr_in_future_heap_reserved(samples[i].addr) == 0) {
            passed = 0u;
        }

        if (samples[i].expected_class == STAGE8A_VMM_LAYOUT_REGION_FUTURE_MAPPING_RESERVED
            && stage8a_vmm_layout_is_addr_in_future_mapping_reserved(samples[i].addr) == 0) {
            passed = 0u;
        }

        if (samples[i].expected_class == STAGE8A_VMM_LAYOUT_REGION_UNCLASSIFIED
            && (stage8a_vmm_layout_is_addr_in_active_identity(samples[i].addr) != 0
                || stage8a_vmm_layout_is_addr_in_future_heap_reserved(samples[i].addr) != 0
                || stage8a_vmm_layout_is_addr_in_future_mapping_reserved(samples[i].addr) != 0)) {
            passed = 0u;
        }

        i++;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 8A: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8A: FAIL\n");
        panic("Stage 8A layout policy failed", 0x000800A0u);
    }
}

static void stage8b_run_vmm_mapping_interface_self_check(void)
{
    const uint32_t test_virtual_page = STAGE8B_TEST_VIRTUAL_PAGE;
    const uint32_t expected_physical_frame = stage7a_paging_frame_addr(STAGE8B_TEST_PHYSICAL_FRAME);
    uint32_t resolved_physical_frame = 0u;
    uint32_t passed = 1u;
    int initial_unmapped = 0;
    int map_result = 0;
    int resolve_result = 0;
    int unmap_result = 0;
    int final_unmapped = 0;

    serial_write_text("custom-os Stage 8B mapping interface begin\n");
    serial_write_label_hex("custom-os Stage 8B test virtual page: ", test_virtual_page);
    serial_write_label_hex("custom-os Stage 8B expected physical frame: ", expected_physical_frame);

    initial_unmapped = (stage8b_vmm_is_page_mapped(test_virtual_page) == 0);
    if (initial_unmapped != 0) {
        serial_write_text("custom-os Stage 8B initial unmapped state: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8B initial unmapped state: FAIL\n");
        passed = 0u;
    }

    map_result = (stage8b_vmm_map_page(test_virtual_page, expected_physical_frame) != 0);
    if (map_result != 0) {
        serial_write_text("custom-os Stage 8B successful map result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8B successful map result: FAIL\n");
        passed = 0u;
    }

    resolve_result = (stage8b_vmm_resolve_page_frame(test_virtual_page, &resolved_physical_frame) != 0)
        && (resolved_physical_frame == expected_physical_frame);
    serial_write_label_hex("custom-os Stage 8B resolved physical frame: ", resolved_physical_frame);
    if (resolve_result == 0) {
        passed = 0u;
    }

    unmap_result = (stage8b_vmm_unmap_page(test_virtual_page) != 0);
    if (unmap_result != 0) {
        serial_write_text("custom-os Stage 8B successful unmap result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8B successful unmap result: FAIL\n");
        passed = 0u;
    }

    final_unmapped = (stage8b_vmm_is_page_mapped(test_virtual_page) == 0);
    if (final_unmapped != 0) {
        serial_write_text("custom-os Stage 8B final unmapped state: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8B final unmapped state: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 8B: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8B: FAIL\n");
        panic("Stage 8B mapping interface failed", 0x000800B0u);
    }
}

static void stage8c_run_kheap_bootstrap_self_check(void)
{
    uint32_t heap_start = 0u;
    uint32_t heap_current = 0u;
    uint32_t heap_end_exclusive = 0u;
    uint32_t heap_mapped_end_exclusive = 0u;
    void* first_allocation = (void*)0;
    void* second_allocation = (void*)0;
    uint32_t first_allocation_addr = 0u;
    uint32_t second_allocation_addr = 0u;
    int alignment_ordering_check = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 8C heap bootstrap begin\n");

    if (stage8c_kheap_bootstrap_init() == 0) {
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    serial_write_label_hex("custom-os Stage 8C heap start: ", heap_start);
    serial_write_label_hex("custom-os Stage 8C heap current: ", heap_current);
    serial_write_label_hex("custom-os Stage 8C heap end exclusive: ", heap_end_exclusive);
    serial_write_label_hex("custom-os Stage 8C heap mapped end exclusive: ", heap_mapped_end_exclusive);

    first_allocation = stage8c_kheap_alloc(24u);
    second_allocation = stage8c_kheap_alloc(40u);
    first_allocation_addr = (uint32_t)(uintptr_t)first_allocation;
    second_allocation_addr = (uint32_t)(uintptr_t)second_allocation;

    serial_write_label_hex("custom-os Stage 8C first allocation result: ", first_allocation_addr);
    serial_write_label_hex("custom-os Stage 8C second allocation result: ", second_allocation_addr);

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    serial_write_label_hex("custom-os Stage 8C heap current post alloc: ", heap_current);
    serial_write_label_hex("custom-os Stage 8C heap mapped end post alloc: ", heap_mapped_end_exclusive);

    alignment_ordering_check =
        (first_allocation != (void*)0)
        && (second_allocation != (void*)0)
        && (first_allocation_addr != second_allocation_addr)
        && ((first_allocation_addr & 0x7u) == 0u)
        && ((second_allocation_addr & 0x7u) == 0u)
        && (first_allocation_addr < second_allocation_addr)
        && (first_allocation_addr >= STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START)
        && (first_allocation_addr < STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE)
        && (second_allocation_addr >= STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START)
        && (second_allocation_addr < STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE)
        && (heap_start == STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START)
        && (heap_end_exclusive == STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE)
        && (heap_mapped_end_exclusive >= heap_start)
        && (heap_mapped_end_exclusive <= heap_end_exclusive)
        && ((heap_mapped_end_exclusive & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u)
        && (heap_current >= heap_start)
        && (heap_current <= heap_mapped_end_exclusive);

    if (alignment_ordering_check != 0) {
        serial_write_text("custom-os Stage 8C alignment/ordering check: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8C alignment/ordering check: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 8C: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8C: FAIL\n");
        panic("Stage 8C heap bootstrap failed", 0x000800C0u);
    }
}

static void stage8d_run_kheap_validation_self_check(void)
{
    static const uint32_t allocation_sizes[STAGE8D_TEST_ALLOC_COUNT] = {
        0x00000080u,
        0x00000180u,
        0x00000300u,
        0x00001000u
    };

    uint32_t heap_start_pre = 0u;
    uint32_t heap_current_pre = 0u;
    uint32_t heap_end_pre = 0u;
    uint32_t heap_mapped_end_pre = 0u;
    uint32_t heap_start_post = 0u;
    uint32_t heap_current_post = 0u;
    uint32_t heap_end_post = 0u;
    uint32_t heap_mapped_end_post = 0u;
    uint32_t allocation_addrs[STAGE8D_TEST_ALLOC_COUNT] = {0u, 0u, 0u, 0u};
    uint32_t i = 0u;
    uint32_t passed = 1u;
    int allocations_non_null = 1;
    int alignment_ordering_ok = 1;
    int mapped_growth_ok = 1;
    int bounds_ok = 1;

    serial_write_text("custom-os Stage 8D allocation validation begin\n");

    if (stage8c_kheap_get_state(
            &heap_start_pre,
            &heap_current_pre,
            &heap_end_pre,
            &heap_mapped_end_pre)
        == 0) {
        passed = 0u;
        allocations_non_null = 0;
        alignment_ordering_ok = 0;
        mapped_growth_ok = 0;
        bounds_ok = 0;
    }

    serial_write_label_hex("custom-os Stage 8D heap state before test start: ", heap_start_pre);
    serial_write_label_hex("custom-os Stage 8D heap state before test current: ", heap_current_pre);
    serial_write_label_hex("custom-os Stage 8D heap state before test end exclusive: ", heap_end_pre);
    serial_write_label_hex("custom-os Stage 8D heap state before test mapped end: ", heap_mapped_end_pre);

    while (i < STAGE8D_TEST_ALLOC_COUNT) {
        allocation_addrs[i] = (uint32_t)(uintptr_t)stage8c_kheap_alloc(allocation_sizes[i]);

        serial_write_label_hex("custom-os Stage 8D allocation index: ", i);
        serial_write_label_hex("custom-os Stage 8D allocation size: ", allocation_sizes[i]);
        serial_write_label_hex("custom-os Stage 8D allocation result: ", allocation_addrs[i]);

        if (allocation_addrs[i] == 0u) {
            allocations_non_null = 0;
        }

        if ((allocation_addrs[i] & 0x7u) != 0u) {
            alignment_ordering_ok = 0;
        }

        if (i > 0u && allocation_addrs[i] <= allocation_addrs[i - 1u]) {
            alignment_ordering_ok = 0;
        }

        i++;
    }

    if (stage8c_kheap_get_state(
            &heap_start_post,
            &heap_current_post,
            &heap_end_post,
            &heap_mapped_end_post)
        == 0) {
        passed = 0u;
        mapped_growth_ok = 0;
        bounds_ok = 0;
    }

    serial_write_label_hex("custom-os Stage 8D heap state after test current: ", heap_current_post);
    serial_write_label_hex("custom-os Stage 8D heap state after test mapped end: ", heap_mapped_end_post);

    mapped_growth_ok =
        (mapped_growth_ok != 0)
        && (heap_mapped_end_post > heap_mapped_end_pre)
        && (heap_mapped_end_post >= (heap_start_post + (2u * STAGE7A_PAGING_PAGE_SIZE)));

    while (bounds_ok != 0 && i > 0u) {
        i--;

        if (allocation_addrs[i] < STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START
            || allocation_addrs[i] >= STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE) {
            bounds_ok = 0;
        }
    }

    bounds_ok =
        (bounds_ok != 0)
        && (heap_start_post == STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_START)
        && (heap_end_post == STAGE8A_VMM_LAYOUT_FUTURE_HEAP_RESERVED_END_EXCLUSIVE)
        && (heap_current_post >= heap_start_post)
        && (heap_current_post <= heap_mapped_end_post)
        && (heap_mapped_end_post <= heap_end_post)
        && ((heap_mapped_end_post & STAGE8A_VMM_LAYOUT_PAGE_MASK_4K) == 0u);

    if (allocations_non_null != 0) {
        serial_write_text("custom-os Stage 8D multiple allocation results: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8D multiple allocation results: FAIL\n");
        passed = 0u;
    }

    if (alignment_ordering_ok != 0) {
        serial_write_text("custom-os Stage 8D ordering and alignment result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8D ordering and alignment result: FAIL\n");
        passed = 0u;
    }

    if (mapped_growth_ok != 0) {
        serial_write_text("custom-os Stage 8D page-growth / mapped-end advancement result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8D page-growth / mapped-end advancement result: FAIL\n");
        passed = 0u;
    }

    if (bounds_ok != 0) {
        serial_write_text("custom-os Stage 8D heap-window bounds result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8D heap-window bounds result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 8D: PASS\n");
    } else {
        serial_write_text("custom-os Stage 8D: FAIL\n");
        panic("Stage 8D heap validation failed", STAGE8D_FAIL_DETAIL);
    }
}

static void stage9a_run_kheap_free_groundwork_self_check(void)
{
    uint32_t heap_start = 0u;
    uint32_t heap_current_before = 0u;
    uint32_t heap_end_exclusive = 0u;
    uint32_t heap_mapped_end_before = 0u;
    uint32_t heap_current_after_free = 0u;
    uint32_t heap_mapped_end_after_free = 0u;
    uint32_t alloc_a = 0u;
    uint32_t alloc_b = 0u;
    uint32_t alloc_c = 0u;
    int valid_free_ok = 0;
    int duplicate_free_reject_ok = 0;
    int invalid_free_reject_ok = 0;
    int null_free_reject_ok = 0;
    int no_reuse_monotonic_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 9A heap free validation begin\n");

    alloc_a = (uint32_t)(uintptr_t)stage8c_kheap_alloc(32u);
    alloc_b = (uint32_t)(uintptr_t)stage8c_kheap_alloc(48u);

    serial_write_label_hex("custom-os Stage 9A alloc A: ", alloc_a);
    serial_write_label_hex("custom-os Stage 9A alloc B: ", alloc_b);

    if (alloc_a == 0u || alloc_b == 0u || alloc_b <= alloc_a) {
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_before,
            &heap_end_exclusive,
            &heap_mapped_end_before)
        == 0) {
        passed = 0u;
    }

    valid_free_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_a) != 0);
    if (valid_free_ok != 0) {
        serial_write_text("custom-os Stage 9A free A accept: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A free A accept: FAIL\n");
        passed = 0u;
    }

    duplicate_free_reject_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_a) == 0);
    if (duplicate_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 9A duplicate free reject: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A duplicate free reject: FAIL\n");
        passed = 0u;
    }

    invalid_free_reject_ok = (stage9a_kheap_free((void*)(uintptr_t)(heap_start + 4u)) == 0);
    if (invalid_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 9A invalid free reject: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A invalid free reject: FAIL\n");
        passed = 0u;
    }

    null_free_reject_ok = (stage9a_kheap_free((void*)0) == 0);
    if (null_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 9A null free reject: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A null free reject: FAIL\n");
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_after_free,
            &heap_end_exclusive,
            &heap_mapped_end_after_free)
        == 0) {
        passed = 0u;
    }

    serial_write_label_hex("custom-os Stage 9A heap current before free: ", heap_current_before);
    serial_write_label_hex("custom-os Stage 9A heap current after free: ", heap_current_after_free);

    if (heap_current_after_free != heap_current_before || heap_mapped_end_after_free != heap_mapped_end_before) {
        passed = 0u;
    }

    alloc_c = (uint32_t)(uintptr_t)stage8c_kheap_alloc(64u);
    serial_write_label_hex("custom-os Stage 9A alloc C after free: ", alloc_c);

    no_reuse_monotonic_ok = (alloc_c != 0u && alloc_c > alloc_b);
    if (no_reuse_monotonic_ok != 0) {
        serial_write_text("custom-os Stage 9A no-reuse monotonic alloc result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A no-reuse monotonic alloc result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 9A: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9A: FAIL\n");
        panic("Stage 9A heap free groundwork failed", STAGE9A_FAIL_DETAIL);
    }
}

static void stage9b_run_kheap_reuse_activation_self_check(void)
{
    uint32_t heap_start = 0u;
    uint32_t heap_current_before_reuse = 0u;
    uint32_t heap_end_exclusive = 0u;
    uint32_t heap_mapped_end_exclusive = 0u;
    uint32_t heap_current_after_reuse = 0u;
    uint32_t alloc_a = 0u;
    uint32_t alloc_b = 0u;
    uint32_t alloc_c = 0u;
    int free_a_ok = 0;
    int reused_address_match_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 9B free-list reuse activation begin\n");

    alloc_a = (uint32_t)(uintptr_t)stage8c_kheap_alloc(56u);
    alloc_b = (uint32_t)(uintptr_t)stage8c_kheap_alloc(88u);

    serial_write_label_hex("custom-os Stage 9B alloc A: ", alloc_a);
    serial_write_label_hex("custom-os Stage 9B alloc B: ", alloc_b);

    if (alloc_a == 0u || alloc_b == 0u || alloc_b <= alloc_a) {
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_before_reuse,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    free_a_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_a) != 0);
    if (free_a_ok != 0) {
        serial_write_text("custom-os Stage 9B free A accept: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9B free A accept: FAIL\n");
        passed = 0u;
    }

    alloc_c = (uint32_t)(uintptr_t)stage8c_kheap_alloc(56u);
    serial_write_label_hex("custom-os Stage 9B reuse allocation result: ", alloc_c);

    reused_address_match_ok = (alloc_c == alloc_a);
    if (reused_address_match_ok != 0) {
        serial_write_text("custom-os Stage 9B reused-address match: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9B reused-address match: FAIL\n");
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_after_reuse,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    if (heap_current_after_reuse != heap_current_before_reuse) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 9B: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9B: FAIL\n");
        panic("Stage 9B heap reuse activation failed", STAGE9B_FAIL_DETAIL);
    }
}

static void stage9c_run_kheap_fragmentation_reuse_self_check(void)
{
    uint32_t heap_start = 0u;
    uint32_t heap_current_before_reuse = 0u;
    uint32_t heap_end_exclusive = 0u;
    uint32_t heap_mapped_end_exclusive = 0u;
    uint32_t heap_current_after_reuse = 0u;
    uint32_t alloc_large = 0u;
    uint32_t alloc_small = 0u;
    uint32_t alloc_leftover = 0u;
    uint32_t expected_leftover_start = 0u;
    int free_large_ok = 0;
    int reused_start_match_ok = 0;
    int leftover_fragment_reuse_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 9C fragmentation-aware reuse begin\n");

    alloc_large = (uint32_t)(uintptr_t)stage8c_kheap_alloc(96u);
    serial_write_label_hex("custom-os Stage 9C alloc large block: ", alloc_large);

    if (alloc_large == 0u) {
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_before_reuse,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    free_large_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_large) != 0);
    if (free_large_ok != 0) {
        serial_write_text("custom-os Stage 9C free large block accept: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9C free large block accept: FAIL\n");
        passed = 0u;
    }

    alloc_small = (uint32_t)(uintptr_t)stage8c_kheap_alloc(40u);
    serial_write_label_hex("custom-os Stage 9C alloc smaller block result: ", alloc_small);

    reused_start_match_ok = (alloc_small == alloc_large);
    if (reused_start_match_ok != 0) {
        serial_write_text("custom-os Stage 9C reused-start match: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9C reused-start match: FAIL\n");
        passed = 0u;
    }

    alloc_leftover = (uint32_t)(uintptr_t)stage8c_kheap_alloc(40u);
    serial_write_label_hex("custom-os Stage 9C leftover fragment allocation: ", alloc_leftover);

    expected_leftover_start = alloc_large + 40u + 16u;
    leftover_fragment_reuse_ok = (alloc_leftover == expected_leftover_start);

    if (leftover_fragment_reuse_ok != 0) {
        serial_write_text("custom-os Stage 9C leftover fragment reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9C leftover fragment reuse result: FAIL\n");
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_after_reuse,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    if (heap_current_after_reuse != heap_current_before_reuse) {
        passed = 0u;
    }

    if (leftover_fragment_reuse_ok == 0) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 9C: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9C: FAIL\n");
        panic("Stage 9C fragmentation reuse failed", STAGE9C_FAIL_DETAIL);
    }
}

static void stage9d_run_kheap_lifecycle_validation_self_check(void)
{
    uint32_t heap_start = 0u;
    uint32_t heap_current_before = 0u;
    uint32_t heap_end_exclusive = 0u;
    uint32_t heap_mapped_end_exclusive = 0u;
    uint32_t heap_current_after = 0u;
    uint32_t alloc_a = 0u;
    uint32_t alloc_b = 0u;
    uint32_t alloc_c = 0u;
    uint32_t alloc_d = 0u;
    uint32_t alloc_e = 0u;
    uint32_t alloc_f = 0u;
    uint32_t expected_leftover_start = 0u;
    int free_b_ok = 0;
    int exact_reuse_ok = 0;
    int free_c_ok = 0;
    int split_reuse_ok = 0;
    int leftover_fragment_reuse_ok = 0;
    int invalid_free_reject_ok = 0;
    int free_d_once_ok = 0;
    int double_free_reject_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 9D heap lifecycle validation begin\n");

    alloc_a = (uint32_t)(uintptr_t)stage8c_kheap_alloc(32u);
    alloc_b = (uint32_t)(uintptr_t)stage8c_kheap_alloc(64u);
    alloc_c = (uint32_t)(uintptr_t)stage8c_kheap_alloc(96u);

    serial_write_label_hex("custom-os Stage 9D alloc A: ", alloc_a);
    serial_write_label_hex("custom-os Stage 9D alloc B: ", alloc_b);
    serial_write_label_hex("custom-os Stage 9D alloc C: ", alloc_c);

    if (alloc_a == 0u || alloc_b == 0u || alloc_c == 0u || alloc_a >= alloc_b || alloc_b >= alloc_c) {
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_before,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    free_b_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_b) != 0);
    if (free_b_ok == 0) {
        passed = 0u;
    }

    alloc_d = (uint32_t)(uintptr_t)stage8c_kheap_alloc(64u);
    serial_write_label_hex("custom-os Stage 9D alloc D exact-size candidate: ", alloc_d);
    exact_reuse_ok = (alloc_d == alloc_b);
    if (exact_reuse_ok != 0) {
        serial_write_text("custom-os Stage 9D exact-size reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D exact-size reuse result: FAIL\n");
        passed = 0u;
    }

    free_c_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_c) != 0);
    if (free_c_ok == 0) {
        passed = 0u;
    }

    alloc_e = (uint32_t)(uintptr_t)stage8c_kheap_alloc(40u);
    serial_write_label_hex("custom-os Stage 9D alloc E split candidate: ", alloc_e);
    split_reuse_ok = (alloc_e == alloc_c);
    if (split_reuse_ok != 0) {
        serial_write_text("custom-os Stage 9D split-reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D split-reuse result: FAIL\n");
        passed = 0u;
    }

    alloc_f = (uint32_t)(uintptr_t)stage8c_kheap_alloc(40u);
    serial_write_label_hex("custom-os Stage 9D alloc F leftover candidate: ", alloc_f);
    expected_leftover_start = alloc_c + 40u + 16u;
    leftover_fragment_reuse_ok = (alloc_f == expected_leftover_start);
    if (leftover_fragment_reuse_ok != 0) {
        serial_write_text("custom-os Stage 9D leftover-fragment reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D leftover-fragment reuse result: FAIL\n");
        passed = 0u;
    }

    invalid_free_reject_ok = (stage9a_kheap_free((void*)(uintptr_t)(heap_start + 4u)) == 0);
    if (invalid_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 9D invalid free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D invalid free rejection result: FAIL\n");
        passed = 0u;
    }

    free_d_once_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_d) != 0);
    if (free_d_once_ok == 0) {
        passed = 0u;
    }

    double_free_reject_ok = (stage9a_kheap_free((void*)(uintptr_t)alloc_d) == 0);
    if (double_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 9D double free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D double free rejection result: FAIL\n");
        passed = 0u;
    }

    if (stage8c_kheap_get_state(
            &heap_start,
            &heap_current_after,
            &heap_end_exclusive,
            &heap_mapped_end_exclusive)
        == 0) {
        passed = 0u;
    }

    if (heap_current_after != heap_current_before) {
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 9D: PASS\n");
    } else {
        serial_write_text("custom-os Stage 9D: FAIL\n");
        panic("Stage 9D heap lifecycle validation failed", STAGE9D_FAIL_DETAIL);
    }
}

static void stage10a_run_kmalloc_interface_self_check(void)
{
    void* alloc_large = (void*)0;
    void* alloc_reused = (void*)0;
    int free_large_ok = 0;
    int exact_reuse_ok = 0;
    int invalid_free_reject_ok = 0;
    int free_once_ok = 0;
    int double_free_reject_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 10A kmalloc interface validation begin\n");

    alloc_large = kmalloc(1536u);
    serial_write_label_hex("custom-os Stage 10A kmalloc alloc large: ", (uint32_t)(uintptr_t)alloc_large);

    if (alloc_large == (void*)0) {
        passed = 0u;
    }

    free_large_ok = (kfree(alloc_large) != 0);
    if (free_large_ok != 0) {
        serial_write_text("custom-os Stage 10A kfree valid free result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10A kfree valid free result: FAIL\n");
        passed = 0u;
    }

    alloc_reused = kmalloc(1536u);
    serial_write_label_hex("custom-os Stage 10A kmalloc exact-reuse candidate: ", (uint32_t)(uintptr_t)alloc_reused);

    exact_reuse_ok = (alloc_reused == alloc_large);
    if (exact_reuse_ok != 0) {
        serial_write_text("custom-os Stage 10A kmalloc exact-reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10A kmalloc exact-reuse result: FAIL\n");
        passed = 0u;
    }

    invalid_free_reject_ok =
        (kfree((void*)(uintptr_t)((uint32_t)(uintptr_t)alloc_reused + 4u)) == 0);
    if (invalid_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 10A kfree invalid free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10A kfree invalid free rejection result: FAIL\n");
        passed = 0u;
    }

    free_once_ok = (kfree(alloc_reused) != 0);
    if (free_once_ok == 0) {
        passed = 0u;
    }

    double_free_reject_ok = (kfree(alloc_reused) == 0);
    if (double_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 10A kfree double free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10A kfree double free rejection result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 10A: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10A: FAIL\n");
        panic("Stage 10A kmalloc interface failed", STAGE10A_FAIL_DETAIL);
    }
}

static void stage10b_run_kmalloc_allocation_semantics_self_check(void)
{
    void* alloc_valid = (void*)0;
    int kmalloc_zero_size_ok = 0;
    int kfree_null_ok = 0;
    int kfree_valid_ok = 0;
    int invalid_free_reject_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 10B allocation semantics begin\n");

    kmalloc_zero_size_ok = (kmalloc(0u) == (void*)0);
    if (kmalloc_zero_size_ok != 0) {
        serial_write_text("custom-os Stage 10B kmalloc zero-size result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10B kmalloc zero-size result: FAIL\n");
        passed = 0u;
    }

    kfree_null_ok = (kfree((void*)0) == 0);
    if (kfree_null_ok != 0) {
        serial_write_text("custom-os Stage 10B kfree null result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10B kfree null result: FAIL\n");
        passed = 0u;
    }

    alloc_valid = kmalloc(128u);
    serial_write_label_hex("custom-os Stage 10B kmalloc valid result: ", (uint32_t)(uintptr_t)alloc_valid);
    if (alloc_valid == (void*)0) {
        passed = 0u;
    }

    kfree_valid_ok = (kfree(alloc_valid) != 0);
    if (kfree_valid_ok != 0) {
        serial_write_text("custom-os Stage 10B kfree valid result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10B kfree valid result: FAIL\n");
        passed = 0u;
    }

    invalid_free_reject_ok =
        (kfree((void*)(uintptr_t)((uint32_t)(uintptr_t)alloc_valid + 4u)) == 0);
    if (invalid_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 10B invalid free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10B invalid free rejection result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 10B: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10B: FAIL\n");
        panic("Stage 10B allocation semantics failed", STAGE10B_FAIL_DETAIL);
    }
}

static void stage10c_run_kmemory_primitives_self_check(void)
{
    uint8_t memset_buffer[16];
    uint8_t memcpy_src[16];
    uint8_t memcpy_dst[16];
    uint8_t move_buffer[8];
    uint8_t expected_move[8];
    uint32_t i = 0u;
    int kmemset_ok = 1;
    int kmemcpy_ok = 1;
    int kmemmove_overlap_ok = 1;
    int kmemcmp_equality_ok = 0;
    int kmemcmp_difference_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 10C memory primitives validation begin\n");

    kmemset(memset_buffer, 0xA5, (uint32_t)sizeof(memset_buffer));
    i = 0u;
    while (i < (uint32_t)sizeof(memset_buffer)) {
        if (memset_buffer[i] != 0xA5u) {
            kmemset_ok = 0;
            break;
        }

        i++;
    }

    if (kmemset_ok != 0) {
        serial_write_text("custom-os Stage 10C kmemset result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C kmemset result: FAIL\n");
        passed = 0u;
    }

    i = 0u;
    while (i < (uint32_t)sizeof(memcpy_src)) {
        memcpy_src[i] = (uint8_t)(i + 1u);
        memcpy_dst[i] = 0u;
        i++;
    }

    kmemcpy(memcpy_dst, memcpy_src, (uint32_t)sizeof(memcpy_src));

    i = 0u;
    while (i < (uint32_t)sizeof(memcpy_src)) {
        if (memcpy_dst[i] != memcpy_src[i]) {
            kmemcpy_ok = 0;
            break;
        }

        i++;
    }

    if (kmemcpy_ok != 0) {
        serial_write_text("custom-os Stage 10C kmemcpy result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C kmemcpy result: FAIL\n");
        passed = 0u;
    }

    i = 0u;
    while (i < (uint32_t)sizeof(move_buffer)) {
        move_buffer[i] = (uint8_t)i;
        expected_move[i] = (uint8_t)i;
        i++;
    }

    expected_move[2] = 0u;
    expected_move[3] = 1u;
    expected_move[4] = 2u;
    expected_move[5] = 3u;
    expected_move[6] = 4u;
    expected_move[7] = 5u;

    kmemmove(&move_buffer[2], &move_buffer[0], 6u);

    i = 0u;
    while (i < (uint32_t)sizeof(move_buffer)) {
        if (move_buffer[i] != expected_move[i]) {
            kmemmove_overlap_ok = 0;
            break;
        }

        i++;
    }

    if (kmemmove_overlap_ok != 0) {
        serial_write_text("custom-os Stage 10C kmemmove overlap result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C kmemmove overlap result: FAIL\n");
        passed = 0u;
    }

    kmemcmp_equality_ok = (kmemcmp(memcpy_src, memcpy_dst, (uint32_t)sizeof(memcpy_src)) == 0);
    if (kmemcmp_equality_ok != 0) {
        serial_write_text("custom-os Stage 10C kmemcmp equality result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C kmemcmp equality result: FAIL\n");
        passed = 0u;
    }

    memcpy_dst[7] ^= 0x01u;
    kmemcmp_difference_ok = (kmemcmp(memcpy_src, memcpy_dst, (uint32_t)sizeof(memcpy_src)) != 0);
    if (kmemcmp_difference_ok != 0) {
        serial_write_text("custom-os Stage 10C kmemcmp difference result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C kmemcmp difference result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 10C: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10C: FAIL\n");
        panic("Stage 10C memory primitives failed", STAGE10C_FAIL_DETAIL);
    }
}

static void stage10d_run_kmalloc_lifecycle_hardening_self_check(void)
{
    uint32_t alloc_a = 0u;
    uint32_t alloc_b = 0u;
    uint32_t alloc_c = 0u;
    uint32_t alloc_d = 0u;
    uint32_t alloc_e = 0u;
    uint32_t alloc_f = 0u;
    uint32_t alloc_split_large = 0u;
    uint32_t expected_leftover = 0u;
    int exact_reuse_ok = 0;
    int split_reuse_ok = 0;
    int leftover_reuse_ok = 0;
    int invalid_free_reject_ok = 0;
    int free_d_once_ok = 0;
    int double_free_reject_ok = 0;
    int alignment_ordering_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 10D allocator hardening validation begin\n");

    alloc_a = (uint32_t)(uintptr_t)kmalloc(32u);
    alloc_b = (uint32_t)(uintptr_t)kmalloc(64u);
    alloc_c = (uint32_t)(uintptr_t)kmalloc(96u);

    serial_write_label_hex("custom-os Stage 10D alloc A: ", alloc_a);
    serial_write_label_hex("custom-os Stage 10D alloc B: ", alloc_b);
    serial_write_label_hex("custom-os Stage 10D alloc C: ", alloc_c);

    alignment_ordering_ok =
        (alloc_a != 0u
            && alloc_b != 0u
            && alloc_c != 0u
            && alloc_a < alloc_b
            && alloc_b < alloc_c
            && (alloc_a & 0x7u) == 0u
            && (alloc_b & 0x7u) == 0u
            && (alloc_c & 0x7u) == 0u);

    if (alignment_ordering_ok == 0) {
        passed = 0u;
    }

    if (kfree((void*)(uintptr_t)alloc_b) == 0) {
        passed = 0u;
    }

    alloc_d = (uint32_t)(uintptr_t)kmalloc(64u);
    serial_write_label_hex("custom-os Stage 10D exact-size reuse candidate: ", alloc_d);

    exact_reuse_ok = (alloc_d == alloc_b);
    if (exact_reuse_ok != 0) {
        serial_write_text("custom-os Stage 10D exact-size reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D exact-size reuse result: FAIL\n");
        passed = 0u;
    }

    alloc_split_large = (uint32_t)(uintptr_t)kmalloc(4096u);
    if (alloc_split_large == 0u) {
        passed = 0u;
    }

    if (kfree((void*)(uintptr_t)alloc_split_large) == 0) {
        passed = 0u;
    }

    alloc_e = (uint32_t)(uintptr_t)kmalloc(2048u);
    serial_write_label_hex("custom-os Stage 10D split-reuse candidate: ", alloc_e);

    split_reuse_ok = (alloc_e == alloc_split_large);
    if (split_reuse_ok != 0) {
        serial_write_text("custom-os Stage 10D split-reuse result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D split-reuse result: FAIL\n");
        passed = 0u;
    }

    alloc_f = (uint32_t)(uintptr_t)kmalloc(1800u);
    serial_write_label_hex("custom-os Stage 10D leftover-fragment candidate: ", alloc_f);

    expected_leftover = alloc_split_large + 2048u + 16u;
    leftover_reuse_ok = (alloc_f == expected_leftover);
    if (leftover_reuse_ok != 0) {
        serial_write_text("custom-os Stage 10D leftover-fragment result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D leftover-fragment result: FAIL\n");
        passed = 0u;
    }

    invalid_free_reject_ok = (kfree((void*)(uintptr_t)(alloc_d + 4u)) == 0);
    if (invalid_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 10D invalid free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D invalid free rejection result: FAIL\n");
        passed = 0u;
    }

    free_d_once_ok = (kfree((void*)(uintptr_t)alloc_d) != 0);
    if (free_d_once_ok == 0) {
        passed = 0u;
    }

    double_free_reject_ok = (kfree((void*)(uintptr_t)alloc_d) == 0);
    if (double_free_reject_ok != 0) {
        serial_write_text("custom-os Stage 10D double free rejection result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D double free rejection result: FAIL\n");
        passed = 0u;
    }

    if (alignment_ordering_ok != 0) {
        serial_write_text("custom-os Stage 10D alignment/ordering result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D alignment/ordering result: FAIL\n");
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 10D: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10D: FAIL\n");
        panic("Stage 10D allocator hardening failed", STAGE10D_FAIL_DETAIL);
    }
}

static void stage10e_run_allocation_api_validation_suite_self_check(void)
{
    uint32_t alloc_a = 0u;
    uint32_t alloc_b = 0u;
    uint32_t alloc_c = 0u;
    uint32_t alloc_exact = 0u;
    uint32_t alloc_split_large = 0u;
    uint32_t alloc_split = 0u;
    uint32_t alloc_leftover = 0u;
    uint32_t expected_leftover = 0u;
    void* continuity_alloc = (void*)0;
    uint8_t memset_buffer[16];
    uint8_t memcpy_src[16];
    uint8_t memcpy_dst[16];
    uint8_t move_buffer[8];
    uint8_t expected_move[8];
    uint32_t i = 0u;
    int lifecycle_ok = 1;
    int wrapper_semantics_ok = 1;
    int memory_primitives_ok = 1;
    int cross_stage_continuity_ok = 1;

    serial_write_text("custom-os Stage 10E allocation API validation suite begin\n");

    alloc_a = (uint32_t)(uintptr_t)kmalloc(48u);
    alloc_b = (uint32_t)(uintptr_t)kmalloc(80u);
    alloc_c = (uint32_t)(uintptr_t)kmalloc(112u);

    if (alloc_a == 0u
        || alloc_b == 0u
        || alloc_c == 0u
        || alloc_a >= alloc_b
        || alloc_b >= alloc_c
        || (alloc_a & 0x7u) != 0u
        || (alloc_b & 0x7u) != 0u
        || (alloc_c & 0x7u) != 0u) {
        lifecycle_ok = 0;
    }

    if (kfree((void*)(uintptr_t)alloc_b) == 0) {
        lifecycle_ok = 0;
    }

    alloc_exact = (uint32_t)(uintptr_t)kmalloc(80u);
    if (alloc_exact != alloc_b) {
        lifecycle_ok = 0;
    }

    alloc_split_large = (uint32_t)(uintptr_t)kmalloc(4096u);
    if (alloc_split_large == 0u) {
        lifecycle_ok = 0;
    }

    if (kfree((void*)(uintptr_t)alloc_split_large) == 0) {
        lifecycle_ok = 0;
    }

    alloc_split = (uint32_t)(uintptr_t)kmalloc(2048u);
    if (alloc_split != alloc_split_large) {
        lifecycle_ok = 0;
    }

    alloc_leftover = (uint32_t)(uintptr_t)kmalloc(1800u);
    expected_leftover = alloc_split_large + 2048u + 16u;
    if (alloc_leftover != expected_leftover) {
        lifecycle_ok = 0;
    }

    if (kfree((void*)(uintptr_t)(alloc_exact + 4u)) != 0) {
        lifecycle_ok = 0;
    }

    if (kfree((void*)(uintptr_t)alloc_exact) == 0) {
        lifecycle_ok = 0;
    }

    if (kfree((void*)(uintptr_t)alloc_exact) != 0) {
        lifecycle_ok = 0;
    }

    if (lifecycle_ok != 0) {
        serial_write_text("custom-os Stage 10E kmalloc/kfree lifecycle result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10E kmalloc/kfree lifecycle result: FAIL\n");
    }

    if (kmalloc(0u) != (void*)0 || kfree((void*)0) != 0) {
        wrapper_semantics_ok = 0;
    }

    if (wrapper_semantics_ok != 0) {
        serial_write_text("custom-os Stage 10E wrapper semantics result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10E wrapper semantics result: FAIL\n");
    }

    kmemset(memset_buffer, 0x5Au, (uint32_t)sizeof(memset_buffer));
    i = 0u;
    while (i < (uint32_t)sizeof(memset_buffer)) {
        if (memset_buffer[i] != 0x5Au) {
            memory_primitives_ok = 0;
            break;
        }

        i++;
    }

    i = 0u;
    while (i < (uint32_t)sizeof(memcpy_src)) {
        memcpy_src[i] = (uint8_t)(i + 3u);
        memcpy_dst[i] = 0u;
        i++;
    }

    kmemcpy(memcpy_dst, memcpy_src, (uint32_t)sizeof(memcpy_src));
    if (kmemcmp(memcpy_src, memcpy_dst, (uint32_t)sizeof(memcpy_src)) != 0) {
        memory_primitives_ok = 0;
    }

    i = 0u;
    while (i < (uint32_t)sizeof(move_buffer)) {
        move_buffer[i] = (uint8_t)i;
        expected_move[i] = (uint8_t)i;
        i++;
    }

    expected_move[2] = 0u;
    expected_move[3] = 1u;
    expected_move[4] = 2u;
    expected_move[5] = 3u;
    expected_move[6] = 4u;
    expected_move[7] = 5u;

    kmemmove(&move_buffer[2], &move_buffer[0], 6u);

    i = 0u;
    while (i < (uint32_t)sizeof(move_buffer)) {
        if (move_buffer[i] != expected_move[i]) {
            memory_primitives_ok = 0;
            break;
        }

        i++;
    }

    memcpy_dst[5] ^= 0x01u;
    if (kmemcmp(memcpy_src, memcpy_dst, (uint32_t)sizeof(memcpy_src)) == 0) {
        memory_primitives_ok = 0;
    }

    if (memory_primitives_ok != 0) {
        serial_write_text("custom-os Stage 10E memory primitives result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10E memory primitives result: FAIL\n");
    }

    continuity_alloc = kmalloc(72u);
    if (continuity_alloc == (void*)0 || kfree(continuity_alloc) == 0) {
        cross_stage_continuity_ok = 0;
    }

    if (lifecycle_ok == 0 || wrapper_semantics_ok == 0 || memory_primitives_ok == 0) {
        cross_stage_continuity_ok = 0;
    }

    if (cross_stage_continuity_ok != 0) {
        serial_write_text("custom-os Stage 10E cross-stage continuity result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10E cross-stage continuity result: FAIL\n");
    }

    if (lifecycle_ok != 0
        && wrapper_semantics_ok != 0
        && memory_primitives_ok != 0
        && cross_stage_continuity_ok != 0) {
        serial_write_text("custom-os Stage 10E: PASS\n");
    } else {
        serial_write_text("custom-os Stage 10E: FAIL\n");
        panic("Stage 10E allocation API suite failed", STAGE10E_FAIL_DETAIL);
    }
}

static void stage11a_run_kobject_groundwork_self_check(void)
{
    const uint32_t expected_id = 0x11A00001u;
    const uint32_t expected_value = 0x0000BEEF;
    struct kobject_node* node = (struct kobject_node*)0;
    int field_init_ok = 0;
    int next_null_ok = 0;
    int free_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 11A kernel object groundwork begin\n");

    node = kobject_node_create(expected_id, expected_value);
    serial_write_label_hex("custom-os Stage 11A object alloc result: ", (uint32_t)(uintptr_t)node);

    field_init_ok =
        (node != (struct kobject_node*)0 && node->id == expected_id && node->value == expected_value);
    if (field_init_ok != 0) {
        serial_write_text("custom-os Stage 11A field init result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11A field init result: FAIL\n");
        passed = 0u;
    }

    next_null_ok = (node != (struct kobject_node*)0 && node->next == (struct kobject_node*)0);
    if (next_null_ok != 0) {
        serial_write_text("custom-os Stage 11A next-null result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11A next-null result: FAIL\n");
        passed = 0u;
    }

    free_ok = (kobject_node_destroy(node) != 0);
    if (free_ok != 0) {
        serial_write_text("custom-os Stage 11A object free result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11A object free result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 11A: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11A: FAIL\n");
        panic("Stage 11A kobject groundwork failed", STAGE11A_FAIL_DETAIL);
    }
}

static void stage11b_run_kobject_linked_structure_self_check(void)
{
    struct kobject_list list;
    struct kobject_node* node1 = (struct kobject_node*)0;
    struct kobject_node* node2 = (struct kobject_node*)0;
    int init_ok = 0;
    int first_append_ok = 0;
    int second_append_ok = 0;
    int linkage_ok = 0;
    int cleanup_ok = 1;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 11B linked structure validation begin\n");

    kobject_list_init(&list);
    init_ok = (list.head == (struct kobject_node*)0
               && list.tail == (struct kobject_node*)0
               && list.count == 0u);

    if (init_ok != 0) {
        serial_write_text("custom-os Stage 11B init result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B init result: FAIL\n");
        passed = 0u;
    }

    node1 = kobject_node_create(0x11B00001u, 0x00001001u);
    first_append_ok = (kobject_list_append(&list, node1) != 0
                       && list.head == node1
                       && list.tail == node1
                       && list.count == 1u
                       && node1 != (struct kobject_node*)0
                       && node1->next == (struct kobject_node*)0);

    if (first_append_ok != 0) {
        serial_write_text("custom-os Stage 11B first append result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B first append result: FAIL\n");
        passed = 0u;
    }

    node2 = kobject_node_create(0x11B00002u, 0x00001002u);
    second_append_ok = (kobject_list_append(&list, node2) != 0
                        && list.head == node1
                        && list.tail == node2
                        && list.count == 2u
                        && node2 != (struct kobject_node*)0
                        && node2->next == (struct kobject_node*)0);

    if (second_append_ok != 0) {
        serial_write_text("custom-os Stage 11B second append result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B second append result: FAIL\n");
        passed = 0u;
    }

    linkage_ok = (node1 != (struct kobject_node*)0
                  && node2 != (struct kobject_node*)0
                  && node1->next == node2);

    if (linkage_ok != 0) {
        serial_write_text("custom-os Stage 11B linkage result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B linkage result: FAIL\n");
        passed = 0u;
    }

    if (node2 != (struct kobject_node*)0 && kobject_node_destroy(node2) == 0) {
        cleanup_ok = 0;
    }

    if (node1 != (struct kobject_node*)0 && kobject_node_destroy(node1) == 0) {
        cleanup_ok = 0;
    }

    if (cleanup_ok != 0) {
        serial_write_text("custom-os Stage 11B cleanup result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B cleanup result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 11B: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11B: FAIL\n");
        panic("Stage 11B linked structure validation failed", STAGE11B_FAIL_DETAIL);
    }
}

static void stage11c_run_kobject_traversal_self_check(void)
{
    struct kobject_list list;
    struct kobject_node* node1 = (struct kobject_node*)0;
    struct kobject_node* node2 = (struct kobject_node*)0;
    struct kobject_node* found = (struct kobject_node*)0;
    int find_first_ok = 0;
    int find_second_ok = 0;
    int find_missing_ok = 0;
    int count_ok = 0;
    int cleanup_ok = 1;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 11C list traversal validation begin\n");

    kobject_list_init(&list);

    node1 = kobject_node_create(0x11C00001u, 0x00002001u);
    node2 = kobject_node_create(0x11C00002u, 0x00002002u);

    if (kobject_list_append(&list, node1) == 0 || kobject_list_append(&list, node2) == 0) {
        passed = 0u;
    }

    found = kobject_list_find_by_id(&list, 0x11C00001u);
    find_first_ok = (found == node1);
    if (find_first_ok != 0) {
        serial_write_text("custom-os Stage 11C find first result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C find first result: FAIL\n");
        passed = 0u;
    }

    found = kobject_list_find_by_id(&list, 0x11C00002u);
    find_second_ok = (found == node2);
    if (find_second_ok != 0) {
        serial_write_text("custom-os Stage 11C find second result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C find second result: FAIL\n");
        passed = 0u;
    }

    found = kobject_list_find_by_id(&list, 0x11C0FFFFu);
    find_missing_ok = (found == (struct kobject_node*)0);
    if (find_missing_ok != 0) {
        serial_write_text("custom-os Stage 11C find missing result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C find missing result: FAIL\n");
        passed = 0u;
    }

    count_ok = (kobject_list_count(&list) == 2u);
    if (count_ok != 0) {
        serial_write_text("custom-os Stage 11C count result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C count result: FAIL\n");
        passed = 0u;
    }

    if (node2 != (struct kobject_node*)0 && kobject_node_destroy(node2) == 0) {
        cleanup_ok = 0;
    }

    if (node1 != (struct kobject_node*)0 && kobject_node_destroy(node1) == 0) {
        cleanup_ok = 0;
    }

    if (cleanup_ok != 0) {
        serial_write_text("custom-os Stage 11C cleanup result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C cleanup result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 11C: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11C: FAIL\n");
        panic("Stage 11C list traversal validation failed", STAGE11C_FAIL_DETAIL);
    }
}

static void stage11d_run_kobject_removal_self_check(void)
{
    struct kobject_list list;
    struct kobject_node* node1 = (struct kobject_node*)0;
    struct kobject_node* node2 = (struct kobject_node*)0;
    struct kobject_node* node3 = (struct kobject_node*)0;
    struct kobject_node* node4 = (struct kobject_node*)0;
    int remove_head_ok = 0;
    int remove_middle_ok = 0;
    int remove_tail_ok = 0;
    int integrity_ok = 0;
    int empty_state_ok = 0;
    uint32_t passed = 1u;

    serial_write_text("custom-os Stage 11D list removal validation begin\n");

    kobject_list_init(&list);

    node1 = kobject_node_create(0x11D00001u, 0x00003001u);
    node2 = kobject_node_create(0x11D00002u, 0x00003002u);
    node3 = kobject_node_create(0x11D00003u, 0x00003003u);
    node4 = kobject_node_create(0x11D00004u, 0x00003004u);

    if (kobject_list_append(&list, node1) == 0
        || kobject_list_append(&list, node2) == 0
        || kobject_list_append(&list, node3) == 0
        || kobject_list_append(&list, node4) == 0) {
        passed = 0u;
    }

    remove_head_ok = (kobject_list_remove_by_id(&list, 0x11D00001u) != 0
                      && kobject_node_destroy(node1) != 0
                      && list.head == node2
                      && list.tail == node4
                      && list.count == 3u);

    if (remove_head_ok != 0) {
        serial_write_text("custom-os Stage 11D remove head result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D remove head result: FAIL\n");
        passed = 0u;
    }

    remove_middle_ok = (kobject_list_remove_by_id(&list, 0x11D00003u) != 0
                        && kobject_node_destroy(node3) != 0
                        && list.head == node2
                        && list.tail == node4
                        && list.count == 2u
                        && node2 != (struct kobject_node*)0
                        && node2->next == node4);

    if (remove_middle_ok != 0) {
        serial_write_text("custom-os Stage 11D remove middle result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D remove middle result: FAIL\n");
        passed = 0u;
    }

    remove_tail_ok = (kobject_list_remove_by_id(&list, 0x11D00004u) != 0
                      && kobject_node_destroy(node4) != 0
                      && list.head == node2
                      && list.tail == node2
                      && list.count == 1u
                      && node2 != (struct kobject_node*)0
                      && node2->next == (struct kobject_node*)0);

    if (remove_tail_ok != 0) {
        serial_write_text("custom-os Stage 11D remove tail result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D remove tail result: FAIL\n");
        passed = 0u;
    }

    integrity_ok = (kobject_list_find_by_id(&list, 0x11D00001u) == (struct kobject_node*)0
                    && kobject_list_find_by_id(&list, 0x11D00003u) == (struct kobject_node*)0
                    && kobject_list_find_by_id(&list, 0x11D00004u) == (struct kobject_node*)0
                    && kobject_list_find_by_id(&list, 0x11D00002u) == node2
                    && kobject_list_count(&list) == 1u
                    && list.head == node2
                    && list.tail == node2);

    if (integrity_ok != 0) {
        serial_write_text("custom-os Stage 11D integrity result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D integrity result: FAIL\n");
        passed = 0u;
    }

    if (kobject_list_remove_by_id(&list, 0x11D00002u) == 0 || kobject_node_destroy(node2) == 0) {
        passed = 0u;
    }

    empty_state_ok = (kobject_list_count(&list) == 0u
                      && list.head == (struct kobject_node*)0
                      && list.tail == (struct kobject_node*)0
                      && kobject_list_find_by_id(&list, 0x11D00002u) == (struct kobject_node*)0);

    if (empty_state_ok != 0) {
        serial_write_text("custom-os Stage 11D empty-state result: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D empty-state result: FAIL\n");
        passed = 0u;
    }

    if (passed != 0u) {
        serial_write_text("custom-os Stage 11D: PASS\n");
    } else {
        serial_write_text("custom-os Stage 11D: FAIL\n");
        panic("Stage 11D list removal validation failed", STAGE11D_FAIL_DETAIL);
    }
}

static void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8u) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

static void pic_set_masks(uint8_t master_mask, uint8_t slave_mask)
{
    outb(PIC1_DATA, master_mask);
    outb(PIC2_DATA, slave_mask);
}

static void pic_remap(uint8_t master_offset, uint8_t slave_offset)
{
    const uint8_t master_mask = inb(PIC1_DATA);
    const uint8_t slave_mask = inb(PIC2_DATA);

    outb(PIC1_COMMAND, (uint8_t)(PIC_ICW1_INIT | PIC_ICW1_ICW4));
    io_wait();
    outb(PIC2_COMMAND, (uint8_t)(PIC_ICW1_INIT | PIC_ICW1_ICW4));
    io_wait();

    outb(PIC1_DATA, master_offset);
    io_wait();
    outb(PIC2_DATA, slave_offset);
    io_wait();

    outb(PIC1_DATA, 0x04u);
    io_wait();
    outb(PIC2_DATA, 0x02u);
    io_wait();

    outb(PIC1_DATA, PIC_ICW4_8086);
    io_wait();
    outb(PIC2_DATA, PIC_ICW4_8086);
    io_wait();

    outb(PIC1_DATA, master_mask);
    outb(PIC2_DATA, slave_mask);
}

static void pit_init(uint32_t target_hz)
{
    const uint32_t divisor = PIT_INPUT_HZ / target_hz;

    outb(PIT_COMMAND, 0x36u);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFFu));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8u) & 0xFFu));
}

static void stage5a_parse_mmap(uint32_t mb2_info_addr)
{
    const uintptr_t info_addr = (uintptr_t)mb2_info_addr;
    const struct multiboot2_info_header* info = (const struct multiboot2_info_header*)info_addr;
    const uint32_t total_size = info->total_size;
    const uintptr_t end_addr = info_addr + (uintptr_t)total_size;
    uintptr_t cursor = info_addr + sizeof(struct multiboot2_info_header);
    uint32_t found_mmap = 0u;
    struct stage5_parse_totals totals;

    if (total_size < sizeof(struct multiboot2_info_header)) {
        panic("invalid Multiboot2 info size", total_size);
    }

    if (end_addr < info_addr) {
        panic("Multiboot2 size overflow", total_size);
    }

    serial_write_text("custom-os Stage 5A: parsing memory map\n");
    stage5b_reset_bookkeeping();
    stage5_parse_totals_reset(&totals);

    while ((cursor + sizeof(struct multiboot2_tag)) <= end_addr) {
        const struct multiboot2_tag* tag = (const struct multiboot2_tag*)cursor;
        const uint32_t aligned_tag_size = align_up_8(tag->size);

        if (tag->size < sizeof(struct multiboot2_tag)) {
            panic("invalid Multiboot2 tag size", tag->size);
        }

        if ((cursor + (uintptr_t)tag->size) > end_addr) {
            panic("truncated Multiboot2 tag", tag->type);
        }

        if (tag->type == MULTIBOOT2_TAG_END) {
            if (tag->size != 8u) {
                panic("invalid Multiboot2 end tag", tag->size);
            }
            break;
        }

        if (tag->type == MULTIBOOT2_TAG_MMAP) {
            const struct multiboot2_mmap_tag* mmap_tag = (const struct multiboot2_mmap_tag*)cursor;
            uintptr_t entry_cursor;
            uintptr_t entry_end;

            found_mmap = 1u;

            if (mmap_tag->size < sizeof(struct multiboot2_mmap_tag)) {
                panic("invalid mmap tag size", mmap_tag->size);
            }

            if (mmap_tag->entry_size < sizeof(struct multiboot2_mmap_entry)) {
                panic("invalid mmap entry size", mmap_tag->entry_size);
            }

            entry_cursor = cursor + sizeof(struct multiboot2_mmap_tag);
            entry_end = cursor + mmap_tag->size;

            while ((entry_cursor + (uintptr_t)mmap_tag->entry_size) <= entry_end) {
                const struct multiboot2_mmap_entry* entry = (const struct multiboot2_mmap_entry*)entry_cursor;

                serial_write_label_hex("custom-os Stage 5A mmap index: ", totals.entry_count);
                serial_write_label_hex64("  base: ", entry->base_addr);
                serial_write_label_hex64("  len : ", entry->length);
                serial_write_label_hex("  type: ", entry->type);

                if (entry->type == MULTIBOOT2_MEMORY_AVAILABLE) {
                    totals.usable_regions++;
                    totals.raw_usable_bytes = add_clamp_u64(totals.raw_usable_bytes, entry->length);
                } else {
                    totals.reserved_regions++;
                    totals.raw_reserved_bytes = add_clamp_u64(totals.raw_reserved_bytes, entry->length);
                }

                stage5b_account_entry(entry);

                totals.entry_count++;
                entry_cursor += mmap_tag->entry_size;
            }
        }

        if (aligned_tag_size == 0u || (cursor + (uintptr_t)aligned_tag_size) > end_addr) {
            panic("invalid aligned tag size", tag->size);
        }

        cursor += aligned_tag_size;
    }

    if (found_mmap == 0u) {
        panic("missing Multiboot2 mmap tag", 0u);
    }

    g_stage5b_memory.raw_usable_bytes = totals.raw_usable_bytes;
    g_stage5b_memory.raw_reserved_bytes = totals.raw_reserved_bytes;
    stage5b_emit_summary(&totals);
}

static const char* exception_name(uint32_t vector)
{
    switch (vector) {
    case 0u:
        return "#DE divide error";
    case 3u:
        return "#BP breakpoint";
    case 6u:
        return "#UD invalid opcode";
    case 13u:
        return "#GP general protection";
    case 14u:
        return "#PF page fault";
    default:
        return "unknown exception";
    }
}

static uint16_t read_cs_selector(void)
{
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

static void idt_set_gate(uint8_t vector, void (*handler)(void), uint16_t selector)
{
    const uint32_t addr = (uint32_t)(uintptr_t)handler;

    g_idt[vector].offset_low = (uint16_t)(addr & 0xFFFFu);
    g_idt[vector].selector = selector;
    g_idt[vector].zero = 0u;
    g_idt[vector].type_attr = IDT_INT_GATE_PRESENT_RING0;
    g_idt[vector].offset_high = (uint16_t)((addr >> 16) & 0xFFFFu);
}

static void idt_clear(void)
{
    uint32_t i = 0;

    while (i < IDT_ENTRIES) {
        g_idt[i].offset_low = 0u;
        g_idt[i].selector = 0u;
        g_idt[i].zero = 0u;
        g_idt[i].type_attr = 0u;
        g_idt[i].offset_high = 0u;
        i++;
    }
}

static void idt_install(void)
{
    const uint16_t code_selector = read_cs_selector();

    idt_clear();

    idt_set_gate(0u, isr_stub_divide_error, code_selector);
    idt_set_gate(3u, isr_stub_breakpoint, code_selector);
    idt_set_gate(6u, isr_stub_invalid_opcode, code_selector);
    idt_set_gate(13u, isr_stub_general_protection, code_selector);
    idt_set_gate(14u, isr_stub_page_fault, code_selector);
    idt_set_gate((uint8_t)(IRQ_BASE_VECTOR + 0u), irq_stub_timer, code_selector);
    idt_set_gate((uint8_t)(IRQ_BASE_VECTOR + 1u), irq_stub_keyboard, code_selector);

    g_idtr.limit = (uint16_t)(sizeof(g_idt) - 1u);
    g_idtr.base = (uint32_t)(uintptr_t)&g_idt[0];

    __asm__ volatile ("lidt %0" : : "m"(g_idtr));
}

void stage3_irq0_handler(void)
{
    g_timer_ticks++;

    if ((g_timer_ticks % PIT_TARGET_HZ) == 0u) {
        serial_write_label_hex("custom-os Stage 6 tick: ", g_timer_ticks);
    }

    pic_send_eoi(0u);
}

void stage4_irq1_handler(void)
{
    const uint8_t scancode = inb(0x60u);

    serial_write_label_hex("custom-os Stage 6 scancode: ", (uint32_t)scancode);
    pic_send_eoi(1u);
}

__attribute__((noreturn)) static void panic(const char* reason, uint32_t detail)
{
    char detail_hex[11];

    format_hex32(detail, detail_hex);
    clear_screen();

    write_text("custom-os Stage 6 PANIC", 0);
    write_text(reason, 1);
    write_text("detail:", 2);
    write_text_at(detail_hex, 2, 8);

    serial_write_text("custom-os Stage 6 PANIC\n");
    serial_write_text(reason);
    serial_write_text("\n");
    serial_write_text("detail: ");
    serial_write_text(detail_hex);
    serial_write_text("\n");

    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

__attribute__((noreturn)) void stage2_exception_dispatch(
    uint32_t vector,
    uint32_t error_code,
    uint32_t eip,
    uint32_t cs,
    uint32_t eflags)
{
    clear_screen();

    write_text("custom-os Stage 2 EXCEPTION", 0);
    write_text(exception_name(vector), 1);
    write_hex_at_row("vector:", 3, 8, vector);
    write_hex_at_row("error :", 4, 8, error_code);
    write_hex_at_row("eip   :", 5, 8, eip);
    write_hex_at_row("cs    :", 6, 8, cs);
    write_hex_at_row("eflags:", 7, 8, eflags);

    serial_write_text("custom-os Stage 2 EXCEPTION\n");
    serial_write_text(exception_name(vector));
    serial_write_text("\n");
    serial_write_label_hex("vector: ", vector);
    serial_write_label_hex("error : ", error_code);
    serial_write_label_hex("eip   : ", eip);
    serial_write_label_hex("cs    : ", cs);
    serial_write_label_hex("eflags: ", eflags);

    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

void stage0_main(uint32_t mb2_magic, uint32_t mb2_info_addr)
{
    clear_screen();
    serial_init();

    write_text("custom-os Stage 6: init start", 0);
    serial_write_text("custom-os v0.11.4 (Stage 11D): init start\n");

#if STAGE1_FORCE_PANIC
    panic("forced panic for Stage 1 test", 0x0000F001u);
#endif

    if (mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("invalid Multiboot2 magic", mb2_magic);
    }

    if (mb2_info_addr == 0u) {
        panic("null Multiboot2 info pointer", mb2_info_addr);
    }

    stage5a_parse_mmap(mb2_info_addr);
    stage5c_prepare_frame_bookkeeping();
    stage5d_run_boot_allocation_test();
    stage7a_run_paging_groundwork_self_check();
    stage7b_run_static_paging_setup_self_check();
    stage7c_run_paging_activation_self_check();
    stage7d_run_identity_validation_self_check();
    stage8a_run_vmm_layout_policy_self_check();
    stage8b_run_vmm_mapping_interface_self_check();
    stage8c_run_kheap_bootstrap_self_check();
    stage8d_run_kheap_validation_self_check();
    stage9a_run_kheap_free_groundwork_self_check();
    stage9b_run_kheap_reuse_activation_self_check();
    stage9c_run_kheap_fragmentation_reuse_self_check();
    stage9d_run_kheap_lifecycle_validation_self_check();
    stage10a_run_kmalloc_interface_self_check();
    stage10b_run_kmalloc_allocation_semantics_self_check();
    stage10c_run_kmemory_primitives_self_check();
    stage10d_run_kmalloc_lifecycle_hardening_self_check();
    stage10e_run_allocation_api_validation_suite_self_check();
    stage11a_run_kobject_groundwork_self_check();
    stage11b_run_kobject_linked_structure_self_check();
    stage11c_run_kobject_traversal_self_check();
    stage11d_run_kobject_removal_self_check();

#if STAGE6D_FORCE_REUSE_TEST
    stage6d_run_reuse_self_test();
#endif

    idt_install();
    pic_remap((uint8_t)IRQ_BASE_VECTOR, (uint8_t)(IRQ_BASE_VECTOR + 8u));
    pic_set_masks(0xFCu, 0xFFu);
    pit_init(PIT_TARGET_HZ);

    write_text("custom-os Stage 6: Multiboot2 handoff OK", 1);
    write_text("custom-os Stage 6: IDT installed", 2);
    write_text("custom-os Stage 6: PIC remapped + PIT started", 3);
    write_text("custom-os Stage 6: deterministic init OK", 4);

    serial_write_text("custom-os Stage 6: Multiboot2 handoff OK\n");
    serial_write_text("custom-os Stage 6: IDT installed\n");
    serial_write_text("custom-os Stage 6: PIC remapped + PIT started\n");
    serial_write_text("custom-os Stage 6: deterministic init OK\n");

    __asm__ volatile ("sti");

#if STAGE2_FORCE_EXCEPTION
    write_text("custom-os Stage 6: triggering INT3 test", 6);
    serial_write_text("custom-os Stage 6: triggering INT3 test\n");
    __asm__ volatile ("int3");
#endif
}
