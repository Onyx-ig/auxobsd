#include "kern.h"
#include "arch.h"
#include "rs.h"

#define HEAP_PAGES 1024   /* 4 MiB kernel heap, managed by Rust */

void kmain(uint32_t mbi)
{
    console_init();
    kprintf(AUXO_NAME " " AUXO_VERSION " (x86_64) - C + Rust + Assembly\n");
    kprintf("boot: long mode OK\n");

    gdt_init();  kprintf("gdt : kernel/user segments + TSS loaded\n");
    idt_init();  kprintf("idt : 48 vectors + int 0x80 syscall gate\n");
    pic_init();

    pmm_init(mbi);
    kprintf("pmm : %lu MiB usable, %lu MiB free\n",
            (unsigned long)(pmm_total_pages() * PAGE_SIZE >> 20),
            (unsigned long)(pmm_free_pages() * PAGE_SIZE >> 20));

    uint64_t heap = pmm_alloc_contig(HEAP_PAGES);
    if (!heap) panic("cannot allocate kernel heap");
    rs_init((uintptr_t)heap, HEAP_PAGES * PAGE_SIZE);
    kprintf("rust: heap + scheduler up (heap at %p)\n", (void *)heap);

    demo_start();
    pit_init(100);
    sti();

    uint8_t shown = 0;
    for (;;) {                       /* pid 0: the idle thread */
        rs_reap();
        if (!shown && kernel_ticks() > 400) { shown = 1; kprintf("\n"); rs_ps(); }
        hlt();
    }
}
