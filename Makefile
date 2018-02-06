
CC          = clang-5.0 -target x86_64-unknown-none
AS          = nasm -felf64
LD          = ld.lld

VM          = qemu-system-x86_64
VMMEM       ?= 64M
#VMOPTS      = -vga std -no-quit -no-reboot -monitor stdio
VMOPTS      = -vga std -no-quit -no-reboot -serial stdio
#VMOPTS      = -vga virtio -no-quit -no-reboot -serial stdio -net nic,model=virtio
VMOPTS      += -m $(VMMEM)

INCLUDE     = -Iinclude -Ikernel

CFLAGS      = $(INCLUDE) -Wall -std=c11                             \
              -nostdlib -nostdinc -ffreestanding                    \
              -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone  \
              -fno-asynchronous-unwind-tables -mcmodel=large

ASFLAGS     =

ifdef RELEASE
CFLAGS		+= -O3
else
CFLAGS      += -g -O0
ASFLAGS     += -g -Fdwarf
endif

LINKSCRIPT  = kernel/link.ld
LDFLAGS     = -nostdlib -T$(LINKSCRIPT) -zmax-page-size=0x1000

SRCDIR      = kernel
MAKEFILE    = Makefile

TARGET      = nightingale.kernel
LIBK		= libk/nightingale-libk.a
ISO         = nightingale.iso

###
# SPECIFICALLY FORBIDS ANY FILES STARTING IN '_'
###
CSRC        = $(shell find $(SRCDIR) -name "[^_]*.c")
CHDR        = $(shell find $(SRCDIR) -name "[^_]*.h")
ASMSRC      = $(shell find $(SRCDIR) -name "[^_]*.asm")

COBJ        = $(CSRC:.c=.c.o)
ASMOBJ      = $(ASMSRC:.asm=.asm.o)

OBJECTS     = $(ASMOBJ) $(COBJ)

SCU_COBJ	= $(SRCDIR)/scu_cobject.o
SCU_ASMOBJ	= $(SRCDIR)/scu_asmobject.o

.PHONY:     all release clean iso run debug dump scu

all: $(TARGET)

NOSCU=I give up on SCU

ifdef NOSCU
$(TARGET): $(OBJECTS) $(MAKEFILE) $(LIBK) $(LINKSCRIPT)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBK)
	rm -f $(TARGET)tmp*
else
$(TARGET): $(SCU_COBJ) $(SCU_ASMOBJ) $(MAKEFILE) $(LIBK)
	$(LD) $(LDFLAGS) -o $(TARGET) $(SCU_COBJ) $(SCU_ASMOBJ) $(LIBK)
	rm -f $(TARGET)tmp*
endif

$(LIBK): libk/libk.c # add libk sources to deps. (or make a build system)
	$(CC) $(CFLAGS) -c libk/libk.c -o $@.o
	ar rcs $@ $@.o

$(SCU_COBJ): $(CSRC) $(CHDR)
	$(CC) $(CFLAGS) -c -DSINGLE_COMPILATION_UNIT kernel/main.c -o $@

$(SCU_ASMOBJ): $(ASMSRC)
	$(AS) $(ASFLAGS) -DSINGLE_COMPILATION_UNIT kernel/boot.asm -o $@

%.asm: 
	# stop it complaining about circular dependancy because %: %.o

%.asm.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# clang is now set to generate .d dependancy files
# they are included to add the dependancy information here
#
# Theoretically this means we will rebuild everything that
# depends on stuff I change.
#
# This is still tenative as to whether I will keep it
include $(shell find . -name "*.d")

%.c.o: %.c %.h
	$(CC) -MD -MF $<.d $(CFLAGS) -c $< -o $@
%.c.o: %.c
	$(CC) -MD -MF $<.d $(CFLAGS) -c $< -o $@

clean:
	rm -f $(shell find . -name "*.o")
	rm -f $(shell find . -name "*.d")
	rm -f $(LIBK)
	rm -f $(TARGET)
	rm -f $(ISO)

$(ISO): $(TARGET)
	mkdir -p isodir/boot/grub
	cp kernel/grub.cfg isodir/boot/grub
	cp $(TARGET) isodir/boot
	grub-mkrescue -o $(ISO) isodir/
	rm -rf isodir

iso: $(ISO)

run: $(ISO)
	$(VM) -cdrom $(ISO) $(VMOPTS)

runint: $(ISO)
	$(VM) -cdrom $(ISO) $(VMOPTS) -d cpu_reset,int

debug: $(ISO)
	$(VM) -cdrom $(ISO) $(VMOPTS) -d cpu_reset -S -s

debugint: $(ISO)
	$(VM) -cdrom $(ISO) $(VMOPTS) -d cpu_reset,int -S -s

dump: $(TARGET)
	# llvm-objdump -x86-asm-symtax=intel -disassemble $(TARGET) | less
	objdump -Mintel -d $(TARGET) | less

dump32: $(TARGET)
	objdump -Mintel,i386 -d $(TARGET) | less

