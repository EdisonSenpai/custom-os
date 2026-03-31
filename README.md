# CustomOS (working title: AnimeOS)

> Current status: Stage 4 - keyboard IRQ groundwork (IRQ1 scancode capture) validated in QEMU

CustomOS is a from-scratch operating system project with a low-level systems focus. The repository is currently at a disciplined Stage 4 baseline with deterministic early initialization, exception groundwork, recurring timer IRQ behavior, and minimal keyboard IRQ handling in place. The long-term direction is an anime-themed experimental OS identity built on top of a technically rigorous kernel foundation.

The goal is to build a clean, well-structured OS from first principles, focusing on correctness, debuggability, and incremental bring-up.

## Architecture snapshot (Stage 4)

- 32-bit protected mode (Multiboot2 entry)
- No paging yet
- Bootloader-provided GDT in use
- IDT installed at runtime
- Exceptions handled via assembly stubs → C dispatcher
- PIC remapped for hardware IRQ vectors
- PIT configured for periodic IRQ0 timer interrupts
- Keyboard IRQ1 routed through IDT and raw scancodes read from port 0x60

## Current baseline (Stage 4)

Implemented and verified:

- Build pipeline via Make.
- Freestanding kernel ELF target.
- GRUB + Multiboot2 bootable ISO generation.
- QEMU run scripts for WSL/Linux and Windows PowerShell.
- Deterministic early init sequence.
- Multiboot2 entry validation.
- Minimal panic path for fatal early boot errors.
- IDT setup.
- Basic CPU exception handling for early bring-up.
- Exception diagnostics output to VGA text mode and COM1 serial.
- PIC remap and IRQ0 unmasking for timer delivery.
- PIT periodic tick configuration.
- Recurring Stage 4 tick output on COM1 serial while idle.
- IRQ1 keyboard interrupt support with raw scancode logging to COM1 serial.

## Stage 4 highlights

- Stage 2 exception coverage remains active with the same deterministic diagnostics path.
- Stage 3 timer groundwork remains active: PIC remap, PIT startup, IRQ0 periodic ticks, and EOI handling.
- Stage 4 adds keyboard IRQ1 support via IDT vector 0x21 with raw scancode capture from port 0x60.
- Timer and keyboard interrupts coexist while the kernel remains alive in the interruptible idle loop.

## Scope boundaries

Implemented now:

- Reproducible build and run flow from source to bootable ISO.
- Stage 4 deterministic startup messages on VGA and serial.
- Minimal panic behavior with explicit halt semantics.
- IDT installation and basic exception diagnostics.
- IRQ0 timer groundwork with PIC/PIT and periodic serial ticks.
- IRQ1 keyboard groundwork with raw scancode serial logging.

Deliberately not implemented yet:

- Scheduler.
- Memory manager.
- Filesystem.
- Drivers.
- Keyboard/input.
- Full x86_64 long-mode runtime.

## Repository map

- docs/: architecture decisions, boot assumptions, milestones, and toolchain notes.
- boot/grub/: GRUB configuration for bootable image generation.
- arch/x86_64/: architecture-specific entry and exception assembly stubs.
- linker/: linker script and memory layout contract.
- kernel/init/: early kernel init and Stage 3 diagnostics.
- build/: shared Make configuration and build targets.
- scripts/: QEMU run wrappers for shell and PowerShell workflows.

## Build commands

### WSL/Linux

```sh
make help
make iso
make run
```

### Windows PowerShell

```powershell
make iso
.\scripts\run-qemu.ps1 -IsoPath out\custom-os.iso
```

## Expected output

### Normal Stage 4 run

VGA and COM1 serial should show:

- custom-os Stage 4: init start
- custom-os Stage 4: Multiboot2 handoff OK
- custom-os Stage 4: IDT installed
- custom-os Stage 4: PIC remapped + PIT started
- custom-os Stage 4: deterministic init OK
- recurring serial lines like custom-os Stage 4 tick: 0x00000064
- keyboard serial lines like custom-os Stage 4 scancode: 0x0000001E

### Forced INT3 exception test

Note: QEMU may appear paused after an exception. This is expected because the CPU halts (cli + hlt).

Run:

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

Expected exception-path markers:

- custom-os Stage 3: triggering INT3 test
- custom-os Stage 2 EXCEPTION
- #BP breakpoint
- vector: 0x00000003
- error : 0x00000000
- eip   : 0x????????
- cs    : 0x????????
- eflags: 0x????????

Note: eip, cs, and eflags are runtime context values and vary by run environment.

## Panic-path self-test (Stage 1 regression check)

Keep this test to ensure the Stage 1 fatal path contract remains intact while Stage 4 evolves.

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

Expected panic output (VGA and serial):

- custom-os Stage 4 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Future direction

- Scheduler bring-up.
- Memory management bring-up.
- User-space experiments.
- Anime-themed UI, terminal, and shell direction at a later stage.

## Key docs

- docs/architecture-decisions.md
- docs/boot-handoff-notes.md
- docs/toolchain-notes.md
- docs/milestone-tracker.md
- docs/milestones/stage-0.md
- docs/milestones/stage-1.md
- docs/milestones/stage-2.md
- docs/milestones/stage-3.md
- docs/milestones/stage-4.md
