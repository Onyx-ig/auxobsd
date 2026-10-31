/* Dual console: COM1 serial (115200 8N1) + VGA text mode 80x25. */
#include "kern.h"
#include "arch.h"

#define COM1 0x3F8
#define VGA_W 80
#define VGA_H 25

static volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
static int cx, cy;
static const uint8_t color = 0x07;

static void serial_init(void)
{
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x01);        /* divisor 1 = 115200 baud */
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);        /* 8N1 */
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static void serial_putc(char c)
{
    for (int spin = 0; spin < 100000 && !(inb(COM1 + 5) & 0x20); spin++)
        ;
    outb(COM1, (uint8_t)c);
}

static void vga_clear(void)
{
    for (int i = 0; i < VGA_W * VGA_H; i++) vga[i] = (uint16_t)(color << 8) | ' ';
    cx = cy = 0;
}

static void vga_putc(char c)
{
    if (c == '\n') { cx = 0; cy++; }
    else if (c == '\r') cx = 0;
    else if (c == '\t') cx = (cx + 8) & ~7;
    else if (c == '\b') { if (cx > 0) { cx--; vga[cy * VGA_W + cx] = (uint16_t)(color << 8) | ' '; } }
    else { vga[cy * VGA_W + cx] = (uint16_t)(color << 8) | (uint8_t)c; cx++; }

    if (cx >= VGA_W) { cx = 0; cy++; }
    if (cy >= VGA_H) {
        for (int i = 0; i < VGA_W * (VGA_H - 1); i++) vga[i] = vga[i + VGA_W];
        for (int i = 0; i < VGA_W; i++) vga[(VGA_H - 1) * VGA_W + i] = (uint16_t)(color << 8) | ' ';
        cy = VGA_H - 1;
    }
}

void console_init(void) { serial_init(); vga_clear(); }

void console_putc(char c)
{
    if (c == '\n') serial_putc('\r');
    serial_putc(c);
    vga_putc(c);
}

void console_write(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) console_putc(buf[i]);
}
