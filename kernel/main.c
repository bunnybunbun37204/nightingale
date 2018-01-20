
#include <string.h>

#include <basic.h>
#include <multiboot2.h>

#include "debug.h"
#include "panic.h"
#include "term/terminal.h"
#include "term/print.h"
#include "cpu/pic.h"
#include "cpu/pit.h"
#include "cpu/portio.h"
#include "memory/allocator.h"
#include "memory/paging.h"
#include "pci.h"

#ifdef SINGLE_COMPILATION_UNIT
#include "pci.c"
#include "cpu/interrupt.c"
#include "cpu/pic.c"
#include "cpu/pit.c"
#include "cpu/uart.c"
#include "memory/allocator.c"
#include "memory/paging.c"
#include "term/print.c"
#include "term/term_serial.c"
#include "term/term_vga.c"
#endif

void kernel_main(usize mb_info, u64 mb_magic) {
    { // initialization
        vga_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
        vga_clear();
        uart_init(COM1);
        printf("Terminal Initialized\n");
        printf("UART Initialized\n");

        remap_pic();
        mask_irq(1);
        mask_irq(2);
        mask_irq(3);
        mask_irq(4);
        mask_irq(5);
        mask_irq(6);
        mask_irq(7);
        mask_irq(8);
        mask_irq(9);
        mask_irq(10);
        mask_irq(11);
        mask_irq(12);
        mask_irq(13);
        mask_irq(14);
        mask_irq(15);
        printf("PIC remapped\n");

        setup_interval_timer(1000);
        printf("Interval Timer Initialized\n");

        uart_enable_interrupt(COM1);
        printf("Serial Interrupts Initialized\n");

        enable_irqs();
        printf("IRQs Enabled\n");

        heap_init();
    }

    { // Multiboot
        printf("Multiboot magic: %p\n", mb_magic);
        printf("Multiboot info*: %p\n", mb_info);

        if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
            panic("Hair on fire - this bootloader isn't multiboot2\n");
        }

        multiboot_tag *tag;
        usize size;

        size = *(u32 *)mb_info;
        printf("Multiboot announced size %i\n", size);

        for (tag = (multiboot_tag *)(mb_info+8);
             tag->type != MULTIBOOT_TAG_TYPE_END;
             tag = (multiboot_tag *)((u8 *)tag + ((tag->size+7) & ~7))) {

            printf("tag type: %i, size: %i\n", tag->type, tag->size);
            switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                multiboot_tag_string *cmd_line_tag = (void *)tag;
                printf("Command line = \"%s\"\n", &cmd_line_tag->string);
                // parse_command_line(&cmd_line_tag->string);
                break;
            }
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                printf ("boot loader name = \"%s\"\n",
                       ((struct multiboot_tag_string *) tag)->string);
                break;
            }
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_mmap_entry *mmap;

                printf("Memory map:\n");

                for (mmap = ((multiboot_tag_mmap *)tag)->entries;
                     (u8 *)mmap < (u8 *)tag + tag->size;
                     mmap = (multiboot_mmap_entry *)((unsigned long) mmap
                         + ((struct multiboot_tag_mmap *) tag)->entry_size)) {

                    printf("base: %p, len: %x (%iM), type %i\n",
                            mmap->addr, mmap->len, mmap->len/(1024*1024), mmap->type);
                }
                break;
            }
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
                multiboot_tag_elf_sections *elf = (void *)tag;
                printf("size    = %i\n", tag->size);
                printf("num     = %i\n", elf->num);
                printf("entsize = %i\n", elf->entsize);
                //panic();
                break;
            }
            default: {
                printf("unhandled\n");
            }
            }
        }

        //panic("exit early so i can read it properly");

        // do the multiboot magic
    }

    { // allocation 
        u8 *alloc_test0 = malloc(16);

        printf("\nalloc_test0 = %x\n", alloc_test0);

        for (i32 i=0; i<16; i++) {
            alloc_test0[i] = i;
        }

        // debug_print_mem(16, alloc_test0-4);
        // debug_dump(alloc_test0);

        free(alloc_test0);
    }

    { // page resolution and mapping
        usize resolved1 = resolve_virtual_to_physical(0x201888);
        printf("resolved vma:%p to pma:%p\n", 0x201888, resolved1);

        map_virtual_to_physical(0x201000, 0x10000);
        usize resolved2 = resolve_virtual_to_physical(0x201888);
        printf("resolved vma:%p to pma:%p\n", 0x201888, resolved2);

        map_virtual_to_physical(0x201000, 0x10000);
        usize resolved3 = resolve_virtual_to_physical(0x201888);
        printf("resolved vma:%p to pma:%p\n", 0x201888, resolved3);

        printf("\n");

        int *pointer_to_be = (int *)0x202000;
        map_virtual_to_physical((usize)pointer_to_be, 0x300000);
        *pointer_to_be = 19;
        printf("pointer_to_be has %i at %p\n", *pointer_to_be, pointer_to_be);
        printf("pointer_to_be lives at %p physically\n", resolve_virtual_to_physical((usize)pointer_to_be));
        // debug_dump(pointer_to_be);'

        map_virtual_to_physical(0x55555000, 0x0);
    }

    { // u128 test
        printf("\n\n");
        u128 x = 0;
        x -= 1;
        // debug_dump(&x); // i can't print this, but i can prove it works this way
    }

    { // memset speed visualization
    }
    
    { // testing length of kernel
        extern usize _kernel_start;
        extern usize _kernel_end;
        /*
        extern usize _kernel_rodata;
        extern usize _kernel_rodata_end;
        extern usize _kernel_text;
        extern usize _kernel_text_end;
        extern usize _kernel_data;
        extern usize _kernel_data_end;
        extern usize _kernel_bss;
        extern usize _kernel_bss_end;
        */

        usize len = (usize)&_kernel_end - (usize)&_kernel_start;
        /*
        usize rodata_len = (usize)&_kernel_rodata_end - (usize)&_kernel_rodata;
        usize text_len = (usize)&_kernel_text_end - (usize)&_kernel_text;
        usize data_len = (usize)&_kernel_data_end - (usize)&_kernel_data;
        usize bss_len = (usize)&_kernel_bss_end - (usize)&_kernel_bss;
        */

        // Why tf does _kernel_start = .; not work in link.ld?
        if ((usize)&_kernel_start == 0x100000) {
            printf("_kernel_start = %p;\n", &_kernel_start);
        } else {
            printf("_kernel_start = %p; // wtf?\n", &_kernel_start);
        }
        printf("_kernel_end   = %p;\n", &_kernel_end);
        printf("\n");
        printf("kernel is %i kilobytes long\n", len / 1024);
        printf("kernel is %x bytes long\n", len);

        /*
        printf("kernel rodata is at %p\n", &_kernel_rodata);
        printf("and is 0x%x long\n", rodata_len);
        printf("kernel text   is at %p\n", &_kernel_text);
        printf("and is 0x%x long\n", text_len);
        printf("kernel data   is at %p\n", &_kernel_data);
        printf("and is 0x%x long\n", data_len);
        printf("kernel bss    is at %p\n", &_kernel_bss);
        printf("and is 0x%x long\n", bss_len);
        */
    }

    { // PCI testing
        printf("\n");
        printf("Discovered PCI devices:\n");

        pci_enumerate_bus_and_print(10, 10);

    }

    { // exit / fail test

        printf("\ntimer_ticks completed = %i\n", timer_ticks);
        if (timer_ticks > 9 && timer_ticks < 99) {
            printf("Theoretically this means we took 0.0%is to execute\n", timer_ticks);
        }

        /* // syscall
        asm volatile ("mov $1, %%rax" ::: "rax");
        asm volatile ("int $0x80");
        */

        /* // can I force a page fault?
        volatile int *x = (int *)0x1000000;
        int (*f_x)() = (void *) 0x10100000;
        f_x();
        *x += 1;
        *x = 1;
        // yes I can
        */

        while (true);
        panic("kernel_main tried to return!\n");
    }
}

