
#include <basic.h>
#include <string.h>
//#include <assert.h> // later, it's in panic for now
#include <multiboot2.h>
#include <debug.h>
#include <panic.h>
#include "print.h"
#include "pmm.h"
#include "vmm.h"
#include "multiboot.h"
#include "malloc.h"
#include "pci.h"
#include "thread.h"
#include <syscalls.h>
#include "vector.h"
#include "uart.h"
#include <rand.h>
#include <mutex.h>
#include <arch/x86/vga.h>
#include <arch/x86/pic.h>
#include <arch/x86/pit.h>
// acpi testing
// #include <arch/x86/acpi.h>
// apic testing
// #include <arch/x86/apic.h>
#include <arch/x86/cpu.h>
#include <net/network.h>
#include <elf.h>
#include <fs/vfs.h>
#include <fs/tarfs.h>

int net_top_id = 0; // TODO: put this somewhere sensible

struct tar_header *initfs;


void kernel_main(uint32_t mb_magic, uintptr_t mb_info) {
    // initialization
    mb_info += VMM_VIRTUAL_OFFSET;

    vmm_early_init();

    vga_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    vga_clear();
    printf("terminal: initialized\n");
    printf("uart: initialized\n");

    rand_add_entropy(0xdeadbeef13378008);
    printf("rand: initialized 'random' generator\n");

    install_isrs();
    printf("idt: interrupts installed\n");

    pic_init(); // leaves everything masked
    pic_irq_unmask(0); // Allow timer though
    printf("pic: remapped and masked\n");

    int timer_interval = 100; // per second
    set_timer_periodic(timer_interval);
    printf("pit: running at %i/s\n", timer_interval);

    x86_uart_enable_interrupt(COM1);
    pic_irq_unmask(4); // Allow serial interrupt
    printf("uart: listening for interrupts\n");
    pic_irq_unmask(1); // Allow keyboard interrupt
    printf("kbrd: listening for interrupts\n");

    if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        panic("Bootloader does not appear to be multiboot2.");
    mb_parse(mb_info);
    mb_mmap_print();

    size_t memory = mb_mmap_total_usable();
    size_t megabytes = memory / (1024 * 1024);
    size_t kilobytes = (memory - (megabytes * 1024 * 1024)) / 1024;
    printf("mmap: total usable memory: %zu (%zuMB + %zuKB)\n",
            memory, megabytes, kilobytes);

    size_t size = *(uint32_t *)mb_info;

    //struct tar_header *initfs = (void *)mb_get_initfs();
    initfs = (void *)mb_get_initfs();
    printf("mb: user init at %#zx\n", initfs);

    // pretty dirty thing - just saying "memory starts after multiboot"...
    // TODO: Cleanup
    //uintptr_t first_free_page = ((uintptr_t)program + 0x10fff) & ~0xfff;

    void *initfs_end = mb_get_initfs_end();
    uintptr_t first_free_page = ((uintptr_t)initfs_end + 0x1fff) & ~0xfff;
    
    first_free_page -= 0xffffffff80000000; // vm-phy offset

    printf("initfs at %#zx\n", initfs);
    printf("pmm: using %#zx as the first physical page\n", first_free_page);

    // So we have something working in the meantime
    pmm_allocator_init(first_free_page, 0x2000000); // TEMPTEMPTEMPTEMP

    init_vfs();
    printf("vfs: filesystem initiated\n");

    network_init();
    printf("network: network initialized\n");

    init_threads();
    printf("threads: process structures initialized\n");

    pci_enumerate_bus_and_print();

    printf("\n");
    printf("********************************\n");
    printf("\n");
    printf("The Nightingale Operating System\n");
    printf("Version " NIGHTINGALE_VERSION "\n");
    printf("\n");
    printf("********************************\n");
    printf("\n");

    enable_irqs();
    printf("cpu: allowing irqs\n");

    Elf64_Ehdr *program = (void *)tarfs_get_file(initfs, "init");

    load_elf(program);
    printf("Starting ring 3 thread at %#zx\n\n", program->e_entry);
    new_user_process(program->e_entry);

    while (true) {
        // system idle thread
        asm volatile ("hlt");
    }

    panic("kernel_main tried to return!");
}

