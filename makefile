
MKDIR = @if [ ! -d $(@D) ] ; then mkdir -p $(@D) ; fi

export ARCH ?= x86_64
export TRIP := $(ARCH)-nightingale
export CC := $(TRIP)-gcc
export LD := $(TRIP)-gcc
export AS := $(TRIP)-gcc
export AR := ar
export NASM := nasm
# grub-mkrescue on Ubuntu 20.04
export MKISO := grub2-mkrescue

STD := -std=c11
WARNING := -Wall -Wextra -Werror -pedantic
DEBUG := -g
OPT := -Og

NGROOT := $(shell pwd)

UINCLUDE :=
KINCLUDE :=

UCFLAGS := $(STD) $(DEBUG) $(OPT) \
	-Wno-builtin-declaration-mismatch \
	-static

ULDFLAGS := -static

VERSION := $(shell git describe --tags)

KCFLAGS := $(STD) $(WARNING) $(DEBUG) $(OPT) \
	-ffreestanding \
	-mno-red-zone \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-nostdlib \
	-fno-asynchronous-unwind-tables \
	-fno-omit-frame-pointer \
	-DNIGHTINGALE_VERSION="\"$(VERSION)\"" \
	-D__kernel__=1 -D_NG=1 \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-unused-function \
 	-Wno-sign-compare \
	-Wno-address-of-packed-member

NASMFLAGS := -g


ifeq ($(ARCH),x86_64)
KLINKSCRIPT := kernel/x86/64/link_hh.ld
NASMFLAGS := -felf64
KCFLAGS += -mcmodel=kernel
else ifeq ($(ARCH),i686)
KLINKSCRIPT := kernel/x86/32/link_hh.ld
NASMFLAGS := -felf32
endif

BUILD := $(NGROOT)/build-$(ARCH)
export SYSROOT := $(NGROOT)/sysroot
export SYSUSR := $(SYSROOT)/usr
export SYSBIN := $(SYSROOT)/usr/bin
export SYSLIB := $(SYSROOT)/usr/lib
export SYSINC := $(SYSROOT)/usr/include

ifdef CI
UCFLAGS += --sysroot="$(SYSROOT)"
ULDFLAGS += --sysroot="$(SYSROOT)"
KCFLAGS += --sysroot="$(SYSROOT)"
endif

ISO := ngos.iso

KLDFLAGS := -nostdlib -T$(KLINKSCRIPT) \
	-L$(BUILD) -zmax-page-size=0x1000 $(DEBUG)

GRUBCFG := $(NGROOT)/kernel/grub.cfg

.PHONY: all clean all-ng make-sysroot
all:
	sh sysroot.sh
	$(MAKE) -C . all-ng

clean:
	rm -rf build-* sysroot
	make -C external/libm clean
	make -C external/lua clean

include linker/make.mk
include libc/libk.mk
include kernel/make.mk

include libc/crt.mk
include libc/libc.mk
include libc/libc.so.mk
include user/programs.mk
include modules/make.mk
include external/make.mk
include sh/make.mk

# Meta-dependancies

$(LINKER):
$(LIBK):
$(LIBC):
$(LIBC_SO):

$(KERNEL): $(LIBK) $(LINKER) $(KLINKSCRIPT)
$(PROGRAMS): $(LIBC) $(CRT)
$(LUA): $(LIBC) $(LIBM)
$(SH): $(LIBC)

### ===

### Init

DIR := $(SYSBIN)
FILES := $(shell find $(DIR) -type f)
OUT := $(BUILD)/init.tar

INIT := $(OUT)

$(OUT): DIR := $(DIR)
$(OUT): $(PROGRAMS) $(MODULES) $(SH)
	$(info tar init.tar)
	@cd $(DIR); tar cf $@ $(notdir $^)

### Raw init (no deps -just sysroot)

.PHONY: rawinit

DIR := $(SYSBIN)
FILES := $(shell find $(DIR) -type f)
OUT := $(BUILD)/init.tar

rawinit: DIR := $(DIR)
rawinit: OUT := $(OUT)
rawinit: FILES := $(FILES)
rawinit:
	$(info tar init.tar)
	@cd $(DIR); tar cf $(OUT) $(notdir $(FILES))

### ISO

OUT := $(ISO)

$(OUT): $(KERNEL) $(INIT) $(GRUBCFG)
	mkdir -p isodir/boot/grub
	cp $(GRUBCFG) isodir/boot/grub
	cp $(KERNEL) isodir/boot/ngk
	cp $(INIT) isodir/boot/initfs
	$(MKISO) -o $(ISO) isodir/
	rm -rf isodir

all-ng: $(ISO)
