// Nightingale-64
// A 64 bit kernel for x86_64
// Copyright (C) 2017, Tyler Philbrick

.intel_syntax noprefix

.section .rodata.multiboot
.set MBLEN, multiboot2_header_end - multiboot2_header
multiboot2_header:
    .long 0xe85250d6
    .long 0 // Intel
    .long MBLEN
    .long 0x100000000 - (0xe85250d6 + 0 + MBLEN) // Checksum

    // end tag
    .long 0, 0, 0
multiboot2_header_end:

.align 16
multiboot1_header:
    .long 0x1BADB002
    .long 0
    .long 0x100000000 - (0x1BADB002 + 0) // Checksum
    .fill 32, 4
multiboot1_header_end:

// Kernel VM offset.  Used to create some low versions of symbols
// for bootstrap
#define VMA 0xFFFFFFFF80000000

// 32 bit bootstrap
.section .text.low
.code32

.global start
start:
    lea esp, stack_top

    push eax
    push ebx

// start_check_long_mode:
    // Test for required cpuid function
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb no64

    // Test for long mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz no64

// start_set_paging:
    // And set up paging
    lea eax, PML4
    mov cr3, eax

    mov eax, cr4
    or eax, 3 << 4  // Enable PAE and huge pages
    mov cr4, eax

    mov ecx, 0xC0000080 // IA32e_EFER MSR
    rdmsr
    or eax, (1 << 8) | (1 << 11) // IA-32e enable | NXE
    wrmsr

    mov eax, cr0
    or eax, 1 << 0  // PE
    or eax, 1 << 31 // PG
    or eax, 1 << 16 // WP
    mov cr0, eax

    mov eax, cr4
    or eax, 1 << 7  // PGE for global pages
    // or eax, 1 << 17 // PCIDE
    mov cr4, eax

// start_enable_fpu:
    fninit

// start_enable_sse:
    mov eax, cr0
    and eax, ~(1 << 2) // Clear CR0.EM
    or eax, 1 << 1     // Set CR0.MP
    mov cr0, eax

    mov eax, cr4
    or eax, 3 << 9     // Set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov cr4, eax

// start_finish_init:
    lgdt [low_gdtp]

    mov dword ptr [0xb8002], 0x2f4b2f4f // OK

    ljmp 8, offset start_64

no64:
    // There is no long mode, print an error and halt
    mov dword ptr [0xb8000], 0x4f6f4f6e // no
    mov dword ptr [0xb8004], 0x4f344f36 // 64
halt:
    hlt
    jmp halt


.section .text.low
.code64
start_64:
    mov edi, [rsp + 4]
    mov esi, [rsp]
    add rsp, 8

    // Don't touch edi or esi again until kernel_main()

    lea rax, start_higher_half
    jmp rax

.section .text
start_higher_half:
    lgdt [gdt64_pointer]    // higher half gdt

    // mov rax, 0xFFFFFFFF80000000
    // add rax, offset boot_p4_mapping
    // mov qword ptr [rax], 0

    // mov rax, 0xFFFFFFFF80000000
    // add rax, offset boot_p3_mapping
    // mov qword ptr [rax], 0

    mov rsp, offset hhstack_top

    mov eax, 0
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax

    mov rax, 0x5f345f365f345f36 // 6464
    mov [0xb8008], rax

// load tss
    lea rax, tss64
    mov [gdt64_tss + 2], ax
    shr rax, 16
    mov [gdt64_tss + 4], al
    shr rax, 8
    mov [gdt64_tss + 7], al
    shr rax, 8
    mov [gdt64_tss + 8], eax


    mov ax, offset TSSDESC
    ltr ax

.extern idt_ptr
    lidt [idt_ptr]

    push 0          // rip = 0
    push 0          // rbp = 0
    mov rbp, rsp    // set up root of backtrace

    // rdi and rsi set above before jump to hh
.extern kernel_main
    call kernel_main
    nop

stop:
    hlt
    jmp stop

cpu_id:
    push rbx
    push rcx
    push rdx
    mov     eax, 1
    cpuid
    shr    ebx, 24
    mov eax, ebx
    pop rdx
    pop rcx
    pop rbx
    ret

.global set_kernel_stack
set_kernel_stack:
    call cpu_id
    cmp eax, 0
    jne 1f
    mov [tss64_stack], rdi
    ret
1:
    mov [core2_tss64_stack], rdi
    ret

.global read_ip
read_ip:
    mov rax, [rsp]
    ret


    .section .text
    .global ap_trampoline
    .code16
    .align 256
ap_trampoline:
    cli
    cld
    mov ax, 0xb800
    mov ds, ax
    mov     word ptr ds:[0x20], 0x2f4f
    mov     word ptr ds:[0x22], 0x2f4b
    mov     word ptr ds:[0x24], 0x2f32
    xor ax, ax
    mov ds, ax
    ljmp    0, 0x8080

    .align 64
_L8040_GDT_table:
    .long 0, 0
    .long 0x0000FFFF, 0x00CF9A00    // flat code
    .long 0x0000FFFF, 0x00CF9200    // flat data
    .long 0, 0
_L8060_GDT_value:
    .word _L8060_GDT_value - _L8040_GDT_table - 1
    .long 0x8040
    .long 0, 0

    .align 64
_L8080:
    xor    ax, ax
    lgdt   [0x8060]
    // mov    eax, 16
    // mov    ds, eax
    // mov    ss, eax
    mov    eax, cr0
    or     eax, 1
    mov    cr0, eax
    ljmp   8, 0x80C0

    .align 64
    .code32
_L80C0:
    mov     word ptr [0xb8028], 0x2f47
    mov     word ptr [0xb802A], 0x2f44
    mov     word ptr [0xb802C], 0x2f4C
    mov esp, 0x8FF0
    lea eax, PML4
    mov cr3, eax

    mov eax, cr4
    or eax, 3 << 4  // Enable PAE and huge pages
    mov cr4, eax

    mov ecx, 0xC0000080 // IA32e_EFER MSR
    rdmsr
    or eax, (1 << 8) | (1 << 11) // IA-32e enable | NXE
    wrmsr

    mov eax, cr0
    or eax, 1 << 0  // PE
    or eax, 1 << 31 // PG
    or eax, 1 << 16 // WP
    mov cr0, eax

    mov eax, cr4
    or eax, 1 << 7  // PGE for global pages
    mov cr4, eax

    fninit

    mov eax, cr0
    and eax, ~(1 << 2) // Clear CR0.EM
    or eax, 1 << 1     // Set CR0.MP
    mov cr0, eax

    mov eax, cr4
    or eax, 3 << 9     // Set CR4.OSFXSR and CR4.OSXMMEXCPT
    mov cr4, eax

    lgdt [0x8220]

    ljmp 8, 0x8180
.align 64
.code64
ap_start_64:
    lea rax, ap_start_higher_half
    jmp rax
.section .text
ap_start_higher_half:
    .code64
    lea rsp, core2_hhstack
    lgdt [core2_gdt64_pointer]    // higher half gdt
    mov eax, 0
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    lea rax, core2_tss64
    mov [core2_gdt64_tss + 2], ax
    shr rax, 16
    mov [core2_gdt64_tss + 4], al
    shr rax, 8
    mov [core2_gdt64_tss + 7], al
    shr rax, 8
    mov [core2_gdt64_tss + 8], eax
    mov ax, offset CORE2_TSSDESC
    ltr ax
.extern idt_ptr
    lidt [idt_ptr]

    push 0          // rip = 0
    push 0          // rbp = 0
    mov rbp, rsp    // set up root of backtrace
.extern ap_kernel_main
    call ap_kernel_main
1:
    cli
    hlt
    jmp 1b

    .align 256
_L8200_GDT64_table:
    .long 0, 0
    .long 0x00000000, 0x00209A00    // flat code
    .long 0x00000000, 0x00209200    // flat data
    .long 0, 0
_L8220_GDT64_value:
    .word _L8220_GDT64_value - _L8200_GDT64_table - 1
    .long 0x8200
    .long 0, 0

.section .bss.low
// Bootstrap low kernel stack
.align 0x10
stack:
    .skip 0x100
stack_top:

.section .bss

.align 0x1000
.global boot_kernel_stack
.global hhstack_guard_page
hhstack_guard_page:
    .skip 0x1000
boot_kernel_stack:
hhstack:
    .skip 0x2000
hhstack_top:

.align 0x10
.global int_stack
int_stack:
    .skip 0x2000
int_stack_top:

.align 0x10
df_stack:
    .skip 0x1000
df_stack_top:

.section .data

tss64:
    .long 0              // reserved 0
tss64_stack:
    .quad int_stack_top  // RSP0
    .quad 0              // RSP1
    .quad 0              // RSP2
    .quad 0              // reserved 0
tss64_ist1:
    .quad df_stack_top
tss64_ist2:
    .quad 0
tss64_ist3:
    .quad 0
tss64_ist4:
    .quad 0
tss64_ist5:
    .quad 0
tss64_ist6:
    .quad 0
tss64_ist7:
    .quad 0
    .quad 0             // reserved 0
    .word 0             // reserved 0
tss64_iomap:
    .word tss64_end - tss64
tss64_end:

.section .data

#define KERNEL_CODE 0x9A
#define KERNEL_DATA 0x92
#define USER_CODE 0xFA
#define USER_DATA 0xF2
#define TSS 0xE9

#define LONG_MODE 0x20

gdt64:
    .quad 0
gdt64_code:
    // See Intel manual section 3.4.5 (Figure 3-8 'Segment Descriptor')

    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte KERNEL_CODE
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
gdt64_data:
    // A valid kernel data segment has to be at kernel_cs + 8 for syscall SS.

    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte KERNEL_DATA
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
gdt64_usrcode:
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte USER_CODE
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
gdt64_usrdata:
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte USER_DATA
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
gdt64_tss:
    // See Intel manual section 7.2.3 (Figure 7-4 'Format of TSS...')
    .word tss64_end - tss64 - 1
    .word 0
    .byte 0
    .byte TSS
    .byte LONG_MODE
    .byte 0
    .quad 0
gdt64_end:
.set GDTLEN, . - gdt64

.set CODEDESC, gdt64_code - gdt64
.set TSSDESC, gdt64_tss - gdt64

gdt64_pointer:
    .word GDTLEN
    .quad gdt64

.section .rodata.low
low_gdt64 = gdt64 - VMA
low_gdtp:
    .word GDTLEN
    .quad low_gdt64

// =  =  =  CORE2  =  =  =
.section .bss

.align 0x10
.global core2_hhstack
core2_hhstack:
    .skip 0x2000
core2_hhstack_top:

.align 0x10
.global core2_int_stack
core2_int_stack:
    .skip 0x2000
core2_int_stack_top:

.align 0x10
core2_df_stack:
    .skip 0x1000
core2_df_stack_top:
.section .data

core2_tss64:
    .long 0              // reserved 0
core2_tss64_stack:
    .quad core2_int_stack_top  // RSP0
    .quad 0              // RSP1
    .quad 0              // RSP2
    .quad 0              // reserved 0
core2_tss64_ist1:
    .quad core2_df_stack_top
core2_tss64_ist2:
    .quad 0
core2_tss64_ist3:
    .quad 0
core2_tss64_ist4:
    .quad 0
core2_tss64_ist5:
    .quad 0
core2_tss64_ist6:
    .quad 0
core2_tss64_ist7:
    .quad 0
    .quad 0             // reserved 0
    .word 0             // reserved 0
core2_tss64_iomap:
    .word core2_tss64_end - core2_tss64
core2_tss64_end:

core2_gdt64:
    .quad 0
core2_gdt64_code:
    // See Intel manual section 3.4.5 (Figure 3-8 'Segment Descriptor')
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte KERNEL_CODE
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
core2_gdt64_data:
    // A valid kernel data segment has to be at kernel_cs + 8 for syscall SS.
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte KERNEL_DATA
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
core2_gdt64_usrcode:
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte USER_CODE
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
core2_gdt64_usrdata:
    .word 0            // segment limit (ignored)
    .word 0            // segment base (ignored)
    .byte 0            // segment base (ignored)
    .byte USER_DATA
    .byte LONG_MODE
    .byte 0            // segment base (ignored)
core2_gdt64_tss:
    // See Intel manual section 7.2.3 (Figure 7-4 'Format of TSS...')
    .word core2_tss64_end - core2_tss64 - 1
    .word 0
    .byte 0
    .byte TSS
    .byte LONG_MODE
    .byte 0
    .quad 0
core2_gdt64_end:
.set CORE2_GDTLEN, . - core2_gdt64

.set CORE2_CODEDESC, core2_gdt64_code - core2_gdt64
.set CORE2_TSSDESC, core2_gdt64_tss - core2_gdt64

core2_gdt64_pointer:
    .word CORE2_GDTLEN
    .quad core2_gdt64

.section .rodata.low
core2_low_gdt64 = core2_gdt64 - VMA
core2_low_gdtp:
    .word CORE2_GDTLEN
    .quad core2_low_gdt64


.section .data.low
.align 0x1000

// Initial paging structures

#define PAGE_PRESENT 0x01
#define PAGE_WRITEABLE 0x02
#define PAGE_USER 0x04
#define PAGE_ISHUGE 0x80
#define PAGE_GLOBAL 0x100

#define PAGE_FLAGS (PAGE_PRESENT | PAGE_WRITEABLE)

.global boot_pt_root
boot_pt_root:
PML4:
boot_p4_mapping:
    .quad PDPT + PAGE_FLAGS
    .fill 255, 8
    // half
    .quad PHYS_PDPT + PAGE_FLAGS
    .fill 254, 8
    .quad PDPT + PAGE_FLAGS
PDPT:
boot_p3_mapping:
    .quad PD + PAGE_FLAGS
    .fill 509, 8
    .quad PD + PAGE_FLAGS
    .quad 0
PD:
    .quad PT0 + PAGE_FLAGS
    .quad PT1 + PAGE_FLAGS
    .quad PT2 + PAGE_FLAGS
    .quad PT3 + PAGE_FLAGS
    .quad PT4 + PAGE_FLAGS
    .quad PT5 + PAGE_FLAGS
    .quad PT6 + PAGE_FLAGS
    .fill 505, 8

PT0: // PT0 covers 000000 -> 200000
    .fill 184, 8
    .quad 0xb8000 + PAGE_FLAGS
    .fill 71, 8
.set PAGE, 0x100000 + PAGE_FLAGS
.rep 256
    .quad PAGE
.set PAGE, PAGE + 0x1000
.endr

.macro pt base
.set PAGE, \base + PAGE_FLAGS
.rep 512
    .quad PAGE
.set PAGE, PAGE + 0x1000
.endr
.endm

PT1: pt 0x200000
PT2: pt 0x400000
PT3: pt 0x600000
PT4: pt 0x800000
PT5: pt 0xA00000
PT6: pt 0xC00000

PHYS_PDPT:
    .quad 0 + PAGE_PRESENT | PAGE_WRITEABLE | PAGE_ISHUGE | PAGE_GLOBAL
    .fill 511, 8

