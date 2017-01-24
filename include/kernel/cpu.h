
#ifndef _CPU_H
#define _CPU_H

#include <stddef.h>
#include <stdint.h>

void gdt_set_gate(size_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install();

/* This defines what the stack looks like after an ISR was running */
struct regs {
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install();

void irq_install();
void irq_install_handler(size_t irq, void (*handler)(struct regs *r));

void timer_phase(int32_t hz);
void timer_install();

void keyboard_echo_handler(struct regs *r);


uint8_t inportb(uint16_t _port);
void outportb(uint16_t _port, uint8_t _data);

#endif


#ifndef _ARCH_I386_KEYBOARD_H
#define _ARCH_I386_KEYBOARD_H

void keyboard_echo_handler(struct regs *r);

#endif // _ARCH_I386_KEYBOARD_H

