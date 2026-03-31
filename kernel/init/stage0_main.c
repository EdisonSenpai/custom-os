#include <stdint.h>

#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_ATTR 0x0F
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COM1_PORT 0x3F8
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u

#ifndef STAGE1_FORCE_PANIC
#define STAGE1_FORCE_PANIC 0
#endif

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

__attribute__((noreturn)) static void panic(const char* reason, uint32_t detail)
{
    char detail_hex[11];

    format_hex32(detail, detail_hex);
    clear_screen();

    write_text("custom-os Stage 1 PANIC", 0);
    write_text(reason, 1);
    write_text("detail:", 2);
    write_text_at(detail_hex, 2, 8);

    serial_write_text("custom-os Stage 1 PANIC\n");
    serial_write_text(reason);
    serial_write_text("\n");
    serial_write_text("detail: ");
    serial_write_text(detail_hex);
    serial_write_text("\n");

    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

void stage0_main(uint32_t mb2_magic, uint32_t mb2_info_addr)
{
    clear_screen();
    serial_init();

    write_text("custom-os Stage 1: init start", 0);
    serial_write_text("custom-os Stage 1: init start\n");

#if STAGE1_FORCE_PANIC
    panic("forced panic for Stage 1 test", 0x0000F001u);
#endif

    if (mb2_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        panic("invalid Multiboot2 magic", mb2_magic);
    }

    if (mb2_info_addr == 0u) {
        panic("null Multiboot2 info pointer", mb2_info_addr);
    }

    write_text("custom-os Stage 1: Multiboot2 handoff OK", 1);
    write_text("custom-os Stage 1: deterministic init OK", 2);

    serial_write_text("custom-os Stage 1: Multiboot2 handoff OK\n");
    serial_write_text("custom-os Stage 1: deterministic init OK\n");
}
