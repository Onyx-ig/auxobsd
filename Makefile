.RECIPEPREFIX := >

CC       ?= gcc
LD       ?= ld
CARGO    ?= cargo
PYTHON   ?= python3
QEMU     ?= qemu-system-x86_64
USE_RUST ?= 1

CFLAGS := -std=gnu11 -O2 -g -Wall -Wextra -m64 -Iinclude -ffreestanding -fno-pic -fno-pie -fno-stack-protector -mno-red-zone -mgeneral-regs-only -mcmodel=small -fno-tree-loop-distribute-patterns
LDFLAGS := -n -nostdlib --no-warn-rwx-segments -z max-page-size=0x1000 -T linker.ld

SRCS_C := $(wildcard kern/*.c) $(wildcard arch/x86_64/*.c)
SRCS_S := $(wildcard arch/x86_64/*.S)
OBJS := $(patsubst %.c,build/%.o,$(SRCS_C)) $(patsubst %.S,build/%.o,$(SRCS_S))

RUSTLIB := rust/target/x86_64-unknown-none/release/libauxo_rs.a

.PHONY: all rust check iso run clean menuconfig

all: build/auxobsd.elf

build/%.o: %.c
>@mkdir -p $(dir $@)
>$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.S
>@mkdir -p $(dir $@)
>$(CC) $(CFLAGS) -c $< -o $@

rust:
>cd rust && $(CARGO) build --release --target x86_64-unknown-none

ifeq ($(USE_RUST),1)

build/auxobsd.elf: $(OBJS) $(RUSTLIB) linker.ld
>$(LD) $(LDFLAGS) -o $@ $(OBJS) $(RUSTLIB)

$(RUSTLIB): $(wildcard rust/src/*.rs) rust/Cargo.toml
>cd rust && $(CARGO) build --release --target x86_64-unknown-none

else

build/auxobsd.elf: $(OBJS) linker.ld
>$(LD) $(LDFLAGS) -o $@ $(OBJS)

endif

check: build/auxobsd.elf
>grub-file --is-x86-multiboot2 $< && echo "multiboot2: OK"

iso: build/auxobsd.elf
>@mkdir -p build
>cp $< iso/boot/auxobsd.elf
>grub-mkrescue -o build/auxobsd.iso iso

run: iso
>$(QEMU) -cdrom build/auxobsd.iso -serial stdio -m 256M

menuconfig:
>$(PYTHON) config/kconfig.py menuconfig

clean:
>rm -rf build iso/boot/auxobsd.elf
>cd rust && $(CARGO) clean
