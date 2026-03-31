#include <stdint.h>

#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_ATTR 0x0F
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COM1_PORT 0x3F8

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

static void write_text(const char* text, uint16_t row)
{
    uint16_t col = 0;
    volatile uint16_t* vga = VGA_TEXT_BUFFER + (row * VGA_WIDTH);

    while (text[col] != '\0' && col < VGA_WIDTH) {
        vga[col] = ((uint16_t)VGA_ATTR << 8) | (uint8_t)text[col];
        col++;
    }
}

void stage0_main(uint32_t mb2_magic, uint32_t mb2_info_addr)
{
    (void)mb2_magic;
    (void)mb2_info_addr;

    clear_screen();
    serial_init();

    write_text("custom-os Stage 0: boot pipeline baseline OK", 0);
    serial_write_text("custom-os Stage 0: boot pipeline baseline OK\n");
}
