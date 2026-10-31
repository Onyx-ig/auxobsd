# AuxoBSD

<p align="center">
  <strong>A BSD-flavoured operating system for x86_64.</strong>
  <br>
  Built from the ground up with Assembly, C, and Rust.
</p>

<p align="center">
  <code>x86_64</code> · <code>no_std</code> · <code>Multiboot2</code> · <code>GRUB</code>
</p>

---

## Overview

AuxoBSD is an experimental BSD-inspired operating system targeting the x86_64 architecture.

Its kernel combines low-level Assembly, C, and Rust to build a modular foundation for memory management, process scheduling, interrupt handling, and system calls.

The project follows a deliberate division of responsibilities:

| Language | Responsibilities | Source |
|---|---|---|
| **Assembly** | Multiboot2 header, CPU mode transition, paging setup, interrupt stubs, context switching, register handling | `arch/x86_64/*.S` |
| **C** | Hardware initialization, console, GDT/TSS, IDT, PIC/PIT, physical memory management, trap dispatch | `kern/*.c`, `arch/x86_64/*.c` |
| **Rust** | Kernel heap, process management, round-robin scheduler, BSD-style syscall dispatcher | `rust/src/*.rs` |

Assembly handles operations requiring direct control over CPU state. C provides the hardware-facing kernel infrastructure, while Rust manages components where memory safety and state consistency are especially important.

## Current Features

- x86_64 long-mode initialization
- Multiboot2 boot protocol support
- 4 GiB identity mapping using 2 MiB pages
- GDT and TSS initialization
- IDT with 48 interrupt vectors
- PIC and PIT initialization
- Serial and VGA console output
- Kernel logging through `kprintf`
- Bitmap-based physical memory manager
- Rust kernel heap allocator
- Basic process table and round-robin scheduler
- Assembly context switching
- BSD-inspired system call interface
- `int 0x80` syscall gate
- Basic kernel thread execution

## BSD Compatibility Design

AuxoBSD adopts selected conventions from BSD systems, particularly FreeBSD.

### System call interface

| System call | Number |
|---|---:|
| `exit` | 1 |
| `write` | 4 |
| `getpid` | 20 |
| `sched_yield` | 331 |

The current syscall interface uses:

- **Entry:** `int 0x80`
- **Arguments:** `rdi`, `rsi`, `rdx`
- **Return value:** `rax`
- **Errors:** negative BSD errno values

The implementation uses BSD errno numbering. For example, `ENOSYS` is 78 rather than Linux's 38.

The process table also follows a BSD-inspired convention: PID 0 represents the idle/swapper process.

## Architecture

### Memory map

The kernel is linked and loaded at physical address `1 MiB`.

The current memory layout uses:

- A 4 GiB identity-mapped address range.
- 2 MiB pages for initial paging.
- A bitmap-based physical memory manager.
- Reserved memory below 1 MiB.
- Reserved kernel and boot-information regions.

A higher-half kernel and per-process virtual address spaces are planned.

### Physical memory manager

The PMM uses a bitmap with one bit per physical frame.

It avoids allocating frames reserved for firmware, the kernel, and boot information.

### Kernel heap

The Rust heap currently uses a contiguous 4 MiB region obtained from the PMM.

Its allocator provides:

- First-fit allocation
- Block splitting
- Block coalescing
- 16-byte alignment

### Scheduling and interrupts

The current implementation targets a single CPU.

Critical sections use `cli` and restore the previous interrupt-enable state afterward.

The PIT runs at 100 Hz, with a 10-tick scheduling quantum. Timer interrupts can trigger preemption inside the IRQ handler, after sending the EOI.

SMP and APIC support are planned.

## Building

### Requirements

- GCC
- GNU Binutils 2.39 or newer
- Rust toolchain
- `x86_64-unknown-none` target
- `make`
- `grub-file`
- `grub-mkrescue`
- `xorriso`
- `qemu-system-x86_64`

Install the Rust target:

```sh
rustup target add x86_64-unknown-none
```

### Build commands

```sh
# Build the kernel
make

# Validate the Multiboot2 image
make check

# Build the ISO and boot it in QEMU
make run

# Build without Rust
make USE_RUST=0
```

The main kernel image is generated at:

```text
build/auxobsd.elf
```

The C and Assembly-only configuration requires compatible `rs_*` stubs to satisfy Rust interface references.

## Boot Output

A successful boot is expected to produce output similar to:

```text
AuxoBSD 0.1.0 (x86_64) - C + Rust + Assembly
boot: long mode OK
gdt : kernel/user segments + TSS loaded
idt : 48 vectors + int 0x80 syscall gate
pmm : ... MiB usable, ... MiB free
rust: heap + scheduler up (heap at 0x...)
  write(2) from a kernel thread via int 0x80
  [worker 1] pid=1 iteration 0 ...
  ...
PID  STATE     TICKS  NAME
0    running   ...    idle
heap: ... KiB used of 4096 KiB
```

This is the expected output format, not a guarantee that every environment will produce an identical result.

## Verification Status

The initial build verification covered the C and Assembly components.

| Check | Status |
|---|---|
| C compilation with `-O2 -Wall -Wextra`
