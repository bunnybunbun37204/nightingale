
#include <basic.h>
#include <print.h>
#include <thread.h>
#include <debug.h>
#include "cpu.h"
#include "cpu32.h"

void print_registers(interrupt_frame* r) {
#define __human_readable_errors // TODO: control in Makefile for tests
#ifdef __human_readable_errors
    uintptr_t cr3 = 0;
    asm volatile ("mov %%cr3, %0" : "=a"(cr3));

    printf("    eax: %8zx    ebx: %8zx\n", r->eax, r->ebx);
    printf("    ecx: %8zx    edx: %8zx\n", r->eax, r->ebx);
    printf("    esi: %8zx    edi: %8zx\n", r->eax, r->ebx);
    printf("    esp: %8zx    ebp: %8zx\n", r->eax, r->ebx);
    printf("    eip: %8zx    efl: [", r->eip);
    printf("%c%c%c%c%c%c] (%zx)\n",
            r->eflags & 0x00000001 ? 'C' : ' ',
            r->eflags & 0x00000004 ? 'P' : ' ',
            r->eflags & 0x00000010 ? 'A' : ' ',
            r->eflags & 0x00000040 ? 'Z' : ' ',
            r->eflags & 0x00000080 ? 'S' : ' ',
            r->eflags & 0x00000200 ? 'I' : ' ',
            r->eflags);
    printf("    cr3: %8zx    pid: %16u\n", cr3, running_thread->pid);

#else /* NOT __human_readable_errors */
    // TODO
#endif /* __human_readable_errors */
}

uintptr_t frame_get(interrupt_frame* r, int reg) {
    switch (reg) {
    case SP:
        return r->user_esp;
    case BP:
        return r->ebp;
    case ARG0:
        return r->eax;
    case ARG1:
        return *(uintptr_t*)(r->user_esp);
    case ARG2:
        return *(uintptr_t*)(r->user_esp + 4);
    case ARG3:
        return *(uintptr_t*)(r->user_esp + 8);
    case ARG4:
        return *(uintptr_t*)(r->user_esp + 12);
    case ARG5:
        return *(uintptr_t*)(r->user_esp + 16);
    case ARG6:
        return *(uintptr_t*)(r->user_esp + 20);
    case RET_VAL:
        return r->eax;
    case RET_ERR:
        return r->ecx;
    case FLAGS:
        return r->eflags;
    case ARGC:
        return r->edi;
    case ARGV:
        return r->esi;
    case ENVP:
        return r->edx;
    case IP:
        return r->eip;
    }
    return -1; // error
}

uintptr_t frame_set(interrupt_frame* r, int reg, uintptr_t value) {
    switch (reg) {
    case SP:
        r->user_esp = value;
        break;
    case BP:
        r->ebp = value;
        break;
    case ARG0:
        r->eax = value;
        break;
    case ARG1:
        *(uintptr_t*)(r->user_esp) = value;
        break;
    case ARG2:
        *(uintptr_t*)(r->user_esp + 4) = value;
        break;
    case ARG3:
        *(uintptr_t*)(r->user_esp + 8) = value;
        break;
    case ARG4:
        *(uintptr_t*)(r->user_esp + 12) = value;
        break;
    case ARG5:
        *(uintptr_t*)(r->user_esp + 16) = value;
        break;
    case ARG6:
        *(uintptr_t*)(r->user_esp + 20) = value;
        break;
    case RET_VAL:
        r->eax = value;
        break;
    case RET_ERR:
        r->ecx = value;
        break;
    case FLAGS:
        r->eflags = value;
        break;
    case ARGC:
        r->edi = value;
        break;
    case ARGV:
        r->esi = value;
        break;
    case ENVP:
        r->edx = value;
        break;
    case IP:
        r->eip = value;
        break;
    }
    return value;
}

