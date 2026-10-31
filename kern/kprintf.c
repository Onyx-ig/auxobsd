/* Minimal printf: %d %i %u %x %X %p %s %c %% with 0-pad, width, l/z length. */
#include "kern.h"
#include "arch.h"

static void putnum(unsigned long long v, unsigned base, int width, int zero, int neg, int upper)
{
    char buf[24];
    int n = 0;
    const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";

    if (v == 0) buf[n++] = '0';
    while (v) { buf[n++] = dig[v % base]; v /= base; }

    int len = n + neg;
    if (!zero) while (width > len) { console_putc(' '); width--; }
    if (neg) console_putc('-');
    if (zero) while (width > len) { console_putc('0'); width--; }
    while (n) console_putc(buf[--n]);
}

int kvprintf(const char *f, va_list ap)
{
    for (; *f; f++) {
        if (*f != '%') { console_putc(*f); continue; }
        f++;
        int zero = 0, width = 0, lng = 0;
        if (*f == '0') { zero = 1; f++; }
        while (*f >= '0' && *f <= '9') { width = width * 10 + (*f - '0'); f++; }
        while (*f == 'l' || *f == 'z') { lng = 1; f++; }

        switch (*f) {
        case 'd': case 'i': {
            long long v = lng ? va_arg(ap, long) : va_arg(ap, int);
            int neg = v < 0;
            putnum(neg ? 0ULL - (unsigned long long)v : (unsigned long long)v, 10, width, zero, neg, 0);
            break;
        }
        case 'u': {
            unsigned long long v = lng ? va_arg(ap, unsigned long) : va_arg(ap, unsigned);
            putnum(v, 10, width, zero, 0, 0);
            break;
        }
        case 'x': case 'X': {
            unsigned long long v = lng ? va_arg(ap, unsigned long) : va_arg(ap, unsigned);
            putnum(v, 16, width, zero, 0, *f == 'X');
            break;
        }
        case 'p':
            console_putc('0'); console_putc('x');
            putnum((uintptr_t)va_arg(ap, void *), 16, 16, 1, 0, 0);
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            int l = (int)strlen(s);
            while (width > l) { console_putc(' '); width--; }
            while (*s) console_putc(*s++);
            break;
        }
        case 'c': console_putc((char)va_arg(ap, int)); break;
        case '%': console_putc('%'); break;
        case 0:   return 0;
        default:  console_putc('%'); console_putc(*f); break;
        }
    }
    return 0;
}

int kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = kvprintf(fmt, ap);
    va_end(ap);
    return r;
}

void panic(const char *fmt, ...)
{
    cli();
    kprintf("\n*** " AUXO_NAME " PANIC: ");
    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);
    kprintf("\nSystem halted.\n");
    for (;;) hlt();
}
