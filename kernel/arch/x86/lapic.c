#include <basic.h>
#include <ng/vmm.h>
#include <stdatomic.h>
#include <stdio.h>
#include <x86/apic.h>
#include <x86/cpu.h>

#define DESTINATION_SELF 1
#define DESTINATION_ALL 2
#define DESTINATION_ALL_OTHER 3

static uint32_t lapic_linear_address = 0xFEE00000;
static uintptr_t lapic_mapped_address;

static void lapic_mmio_w(int reg, uint32_t value)
{
    atomic_store((_Atomic uint32_t *)(lapic_mapped_address + reg), value);
}

static uint32_t lapic_mmio_r(int reg)
{
    return atomic_load((_Atomic uint32_t *)(lapic_mapped_address + reg));
}

static void lapic_init_timer();

void lapic_init()
{
    if (!lapic_mapped_address)
        lapic_mapped_address = vmm_mapobj_iwi(lapic_linear_address, 0x1000);

    uint64_t lapic_base_msr = rdmsr(IA32_APIC_BASE);
    wrmsr(IA32_APIC_BASE, lapic_base_msr | (1 << 11));

    lapic_mmio_w(LAPIC_ESR, 0);
    lapic_mmio_w(LAPIC_SVR, 0x1FF);

    lapic_mmio_w(LAPIC_LVT_LINT0, 0x00008700);
    lapic_mmio_w(LAPIC_LVT_LINT1, 0x00008400);

    lapic_init_timer();
}

void lapic_eoi(int interrupt_number)
{
    (void)interrupt_number;
    lapic_mmio_w(LAPIC_EOI, 0);
}

static void lapic_await_delivery()
{
    while (lapic_mmio_r(LAPIC_ICR1) & (1 << 12)) {
        asm volatile("pause");
    }
}

static void lapic_send_ipi_raw(uint32_t icr, int destination_processor)
{
    lapic_mmio_w(LAPIC_ICR2, destination_processor << 24);
    lapic_mmio_w(LAPIC_ICR1, icr);
    delay(1000);
    lapic_await_delivery();
}

void lapic_send_init(int destination_processor)
{
    uint32_t command = ((IPI_INIT << 8) |
        // (1 << 12) |
        (1 << 14) | // assert
        (1 << 15) | // level triggered
        (3 << 18) // all except self
    );
    lapic_send_ipi_raw(command, 1);
    delay(1000);
    command = ((IPI_INIT << 8) | (1 << 15) | (3 << 18));
    lapic_send_ipi_raw(command, 1);
}

void lapic_send_ipi(int type, int vector, int destination_processor)
{
    uint32_t command = (vector | (type << 8));
    lapic_send_ipi_raw(command, destination_processor);
}

static void lapic_init_timer(void)
{
    lapic_mmio_w(LAPIC_LVT_TIMER, 0x20020);
    lapic_mmio_w(LAPIC_TIMER_DCR, 0);
    lapic_mmio_w(LAPIC_TIMER_ICR, 100000);
}
