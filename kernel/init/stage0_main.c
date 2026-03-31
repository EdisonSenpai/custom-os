#include <stdint.h>

#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_ATTR 0x0F
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COM1_PORT 0x3F8
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u
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

#ifndef STAGE1_FORCE_PANIC
#define STAGE1_FORCE_PANIC 0
#endif

#ifndef STAGE2_FORCE_EXCEPTION
#define STAGE2_FORCE_EXCEPTION 0
#endif

static volatile uint32_t g_timer_ticks = 0u;

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
        serial_write_label_hex("custom-os Stage 4 tick: ", g_timer_ticks);
    }

    pic_send_eoi(0u);
}

void stage4_irq1_handler(void)
{
    const uint8_t scancode = inb(0x60u);

    serial_write_label_hex("custom-os Stage 4 scancode: ", (uint32_t)scancode);
    pic_send_eoi(1u);
}

__attribute__((noreturn)) static void panic(const char* reason, uint32_t detail)
{
    char detail_hex[11];

    format_hex32(detail, detail_hex);
    clear_screen();

    write_text("custom-os Stage 4 PANIC", 0);
    write_text(reason, 1);
    write_text("detail:", 2);
    write_text_at(detail_hex, 2, 8);

    serial_write_text("custom-os Stage 4 PANIC\n");
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

    write_text("custom-os Stage 4: init start", 0);
    serial_write_text("custom-os Stage 4: init start\n");

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
    pic_remap((uint8_t)IRQ_BASE_VECTOR, (uint8_t)(IRQ_BASE_VECTOR + 8u));
    pic_set_masks(0xFCu, 0xFFu);
    pit_init(PIT_TARGET_HZ);

    write_text("custom-os Stage 4: Multiboot2 handoff OK", 1);
    write_text("custom-os Stage 4: IDT installed", 2);
    write_text("custom-os Stage 4: PIC remapped + PIT started", 3);
    write_text("custom-os Stage 4: deterministic init OK", 4);

    serial_write_text("custom-os Stage 4: Multiboot2 handoff OK\n");
    serial_write_text("custom-os Stage 4: IDT installed\n");
    serial_write_text("custom-os Stage 4: PIC remapped + PIT started\n");
    serial_write_text("custom-os Stage 4: deterministic init OK\n");

    __asm__ volatile ("sti");

#if STAGE2_FORCE_EXCEPTION
    write_text("custom-os Stage 4: triggering INT3 test", 6);
    serial_write_text("custom-os Stage 4: triggering INT3 test\n");
    __asm__ volatile ("int3");
#endif
}
