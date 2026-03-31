#include <stdint.h>

#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_ATTR 0x0F
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COM1_PORT 0x3F8
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u
#define IDT_ENTRIES 256u
#define IDT_INT_GATE_PRESENT_RING0 0x8Eu

#ifndef STAGE1_FORCE_PANIC
#define STAGE1_FORCE_PANIC 0
#endif

#ifndef STAGE2_FORCE_EXCEPTION
#define STAGE2_FORCE_EXCEPTION 0
#endif

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

extern void isr_stub_divide_error(void);
extern void isr_stub_breakpoint(void);
extern void isr_stub_invalid_opcode(void);
extern void isr_stub_general_protection(void);
extern void isr_stub_page_fault(void);

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

    g_idtr.limit = (uint16_t)(sizeof(g_idt) - 1u);
    g_idtr.base = (uint32_t)(uintptr_t)&g_idt[0];

    __asm__ volatile ("lidt %0" : : "m"(g_idtr));
}

__attribute__((noreturn)) static void panic(const char* reason, uint32_t detail)
{
    char detail_hex[11];

    format_hex32(detail, detail_hex);
    clear_screen();

    write_text("custom-os Stage 2 PANIC", 0);
    write_text(reason, 1);
    write_text("detail:", 2);
    write_text_at(detail_hex, 2, 8);

    serial_write_text("custom-os Stage 2 PANIC\n");
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

    write_text("custom-os Stage 2: init start", 0);
    serial_write_text("custom-os Stage 2: init start\n");

#if STAGE1_FORCE_PANIC
    panic("forced panic for Stage 1 test", 0x0000F001u);
#endif

    if (mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("invalid Multiboot2 magic", mb2_magic);
    }

    if (mb2_info_addr == 0u) {
        panic("null Multiboot2 info pointer", mb2_info_addr);
    }

    idt_install();

    write_text("custom-os Stage 2: Multiboot2 handoff OK", 1);
    write_text("custom-os Stage 2: IDT installed", 2);
    write_text("custom-os Stage 2: deterministic init OK", 3);

    serial_write_text("custom-os Stage 2: Multiboot2 handoff OK\n");
    serial_write_text("custom-os Stage 2: IDT installed\n");
    serial_write_text("custom-os Stage 2: deterministic init OK\n");

#if STAGE2_FORCE_EXCEPTION
    write_text("custom-os Stage 2: triggering INT3 test", 5);
    serial_write_text("custom-os Stage 2: triggering INT3 test\n");
    __asm__ volatile ("int3");
#endif
}
