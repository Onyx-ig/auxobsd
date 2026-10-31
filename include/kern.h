#ifndef AUXO_KERN_H
#define AUXO_KERN_H
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define AUXO_NAME    "AuxoBSD"
#define AUXO_VERSION "0.1.0"
#define PAGE_SIZE    4096UL

/* console.c / kprintf.c */
void console_init(void);
void console_putc(char c);
void console_write(const char *buf, size_t len);
int  kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int  kvprintf(const char *fmt, va_list ap);
void panic(const char *fmt, ...) __attribute__((noreturn, format(printf, 1, 2)));

/* string.c */
void  *memset(void *d, int c, size_t n);
void  *memcpy(void *d, const void *s, size_t n);
void  *memmove(void *d, const void *s, size_t n);
int    memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);

/* pmm.c */
void     pmm_init(uint32_t mbi_addr);
uint64_t pmm_alloc_page(void);
uint64_t pmm_alloc_contig(size_t pages);
void     pmm_free_page(uint64_t addr);
size_t   pmm_free_pages(void);
size_t   pmm_total_pages(void);

/* demo.c */
void demo_start(void);

#endif
