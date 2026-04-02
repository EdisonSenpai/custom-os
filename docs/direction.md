# Status note:

This document represents long-term architectural direction and early planning.

The authoritative source for current staged progress is:

- docs/milestone-tracker.md
- docs/milestones/*

If any conflict exists, follow the milestone tracker and milestone documents.

## 1) concise project definition

Build a from-scratch, monolithic x86_64 operating system for learning and long-term engineering growth, optimized for correctness, debuggability, and visible incremental progress rather than fast feature count. The project targets emulator-first development in QEMU, starts with a text console experience, and grows toward a usable system with polished interaction patterns inspired by Windows-level usability (not binary compatibility) and Unix-like composable tools plus permissions/filesystem philosophy.

## 2) recommended architecture choices

- CPU and platform: x86_64 only for initial scope; defer other architectures until core kernel is stable.
- Toolchain: freestanding cross-toolchain (x86_64-elf GCC/binutils), no host libc assumptions.
- Language split: C for nearly all kernel code; assembly only for unavoidable boundaries (early entry, interrupt stubs, context switch glue).
- Kernel runtime: no dynamic runtime magic in early boot; explicit initialization order and explicit ownership of global state.
- Logging and debug: serial console (COM1) as first-class debug channel from earliest boot phase; VGA text output for visible local progress.
- Memory strategy: staged bring-up (boot-time bump allocator, then physical page frame allocator, then kernel heap, then virtual memory refinement).
- Interrupt and timing strategy: start with basic IDT and exception handling, then PIC/APIC timer-based ticks as scheduler work begins.
- User/kernel API direction: minimal syscall surface first, designed around stable structs with fixed-width types and explicit versioning where needed.

## 3) chosen boot approach

Use GRUB with Multiboot2 as the first boot path.

- Why this choice: it removes early bootloader complexity so effort goes into kernel fundamentals.
- Entry contract: GRUB loads the kernel image and provides Multiboot2 information; a minimal architecture entry stage validates the handoff, establishes a known stack and paging state, then transfers into C kernel initialization.
- Boot assumptions to document in code and docs:
  - CPU mode at entry and transition steps into long mode.
  - Interrupt state on entry.
  - Ownership and lifetime of Multiboot2 memory map and tags.
  - Kernel virtual and physical layout expectations from the linker script.
- Validation: inspect symbols and sections (for example via readelf/objdump) and run QEMU boot smoke tests on every meaningful boot-path change.

## 4) kernel model recommendation

Use a monolithic kernel with strict internal modular boundaries.

- Rationale for solo development:
  - Easier end-to-end debugging and fewer IPC/ABI layers early on.
  - Faster incremental progress for core OS topics (interrupts, memory, scheduling, VFS).
- Risk control:
  - Keep subsystem interfaces narrow and explicit.
  - Treat ABI-like boundaries (syscalls, interrupt frames, packed structures) as high-risk and version/document them.
  - Prefer deterministic behavior over abstraction-heavy design in early stages.

## 5) realistic staged milestone roadmap

Stage 0: Build and boot pipeline baseline

- Scope: cross-toolchain setup, linker script, kernel image generation, GRUB ISO flow, QEMU run command.
- Acceptance criteria:
  - One command builds bootable image reproducibly.
  - QEMU boots to kernel entry and shows both serial and on-screen text marker.

Stage 1: Early kernel initialization and panic path

- Scope: deterministic init order, boot info parsing, panic handler, basic diagnostics.
- Acceptance criteria:
  - Kernel prints validated boot info summary (memory regions, cmdline presence).
  - Controlled panic prints file/line/reason and halts reliably.

Stage 2: CPU exceptions and interrupt baseline

- Scope: IDT setup, exception handlers, timer interrupt tick.
- Acceptance criteria:
  - Divide-by-zero and page-fault tests route to handlers with register dump.
  - Periodic timer tick increments visible counter without instability for several minutes.

Stage 3: Memory management foundation

- Scope: physical frame allocator and a minimal kernel heap API.
- Acceptance criteria:
  - Frame allocator passes allocate/free stress loop under QEMU.
  - Heap allocations of varied sizes succeed with integrity checks enabled.

Stage 4: Process and scheduling minimum viable model

- Scope: kernel task abstraction, context switch glue, round-robin scheduler.
- Acceptance criteria:
  - At least two independent tasks alternate execution for sustained runtime.
  - Scheduler tick and task state transitions are observable in debug logs.

Stage 5: Syscall path and user mode bring-up

- Scope: minimal userspace binary format loading, syscall entry/exit, small libc-like user stubs.
- Acceptance criteria:
  - User program can print to console through syscall path.
  - Invalid syscall number and bad user pointer are rejected safely.

Stage 6: Filesystem and permissions model (Unix-like philosophy)

- Scope: simple VFS layer, one concrete filesystem (start with in-memory or simple disk format), UID/GID-style permission checks.
- Acceptance criteria:
  - Create/read/write/list operations work through VFS API.
  - Permission checks deny disallowed access and permit expected access in tests.

Stage 7: Composable userland tools and shell

- Scope: basic shell, argument parsing, pipelines or redirection primitives, small utility set.
- Acceptance criteria:
  - At least three tools compose in a pipeline-like workflow.
  - Shell script with multiple commands runs deterministically.

Stage 8: Framebuffer graphics follow-on (after text mode stability)

- Scope: linear framebuffer init via boot info, primitive drawing, text rendering in graphics mode.
- Acceptance criteria:
  - Kernel can switch from text-first path to framebuffer rendering path behind a boot flag.
  - Panic/debug output still has a reliable serial fallback.

## 6) main subsystem list

- Boot and handoff: GRUB/Multiboot2 integration, entry validation, boot info structures.
- Architecture layer (x86_64): GDT/IDT, interrupt stubs, timer, context switch glue.
- Kernel core: init sequencing, panic/assert, logging, configuration flags.
- Memory management: physical memory manager, virtual memory mapping, kernel heap.
- Scheduler and tasks: task model, run queue, timer-driven scheduling.
- Syscall and user boundary: syscall dispatcher, validation, user ABI structs.
- Storage and VFS: block device abstraction, VFS nodes, permissions model.
- Console and I/O: serial, VGA text console, later framebuffer console.
- Userland base: init process, shell, core composable tools.
- Test and diagnostics: boot smoke tests, subsystem self-tests, artifact inspection scripts.

## 7) key risks and tradeoffs

GRUB + Multiboot2 tradeoff:

- Benefit: faster progress into kernel logic.
- Cost: less direct learning of raw firmware bootstrapping initially.

Monolithic kernel tradeoff:

- Benefit: simpler integration and performance.
- Cost: faults can have larger blast radius without strict module discipline.

C-first with minimal assembly tradeoff:

- Benefit: maintainability and lower cognitive load.
- Cost: some low-level behavior still requires careful assembly and ABI auditing.

QEMU-first tradeoff:

- Benefit: rapid iteration and reproducible debugging.
- Cost: hardware edge cases may be discovered late if real-hardware tests are postponed too long.

Text-first tradeoff:

- Benefit: quicker bring-up and debugging clarity.
- Cost: usability polish is deferred until framebuffer/userland maturity.

Solo scope risk:

- Broad subsystem surface can cause stall.
- Mitigation: enforce milestone acceptance criteria and avoid parallel unfinished subsystems.

## 8) exact repository and folder structure to start with

custom-os/
  README.md
  LICENSE
  Makefile
  .gitignore
  docs/
    direction.md
    milestones.md
  boot/
    grub/
      grub.cfg
    multiboot2_header.S
  arch/
    x86_64/
      entry.S
      idt_stubs.S
      gdt.c
      idt.c
      cpu.h
  kernel/
    init/
      kernel_main.c
    core/
      panic.c
      log.c
    console/
      vga_text.c
      serial.c
    mm/
      pmm.c
      kheap.c
  include/
    kernel/
      types.h
      bootinfo.h
      panic.h
      log.h
      console.h
      mm.h
  linker/
    x86_64.ld
  scripts/
    run-qemu.ps1
    run-qemu.sh
    inspect-image.sh
