# CustomOS (working title: AnimeOS)

> Current status: Stage 2 – IDT + exception handling validated in QEMU

CustomOS is a from-scratch operating system project with a low-level systems focus. The repository is currently at a disciplined Stage 2 baseline with deterministic early initialization and exception groundwork in place. The long-term direction is an anime-themed experimental OS identity built on top of a technically rigorous kernel foundation.

The goal is to build a clean, well-structured OS from first principles, focusing on correctness, debuggability, and incremental bring-up.

## Current baseline (Stage 2)

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

## Architecture snapshot (Stage 2)

- 32-bit protected mode (Multiboot2 entry)
- No paging yet
- Bootloader-provided GDT in use
- IDT installed at runtime
- Exceptions handled via assembly stubs → C dispatcher

## Stage 2 highlights

- Stage 2 introduces explicit IDT setup in the early init path.
- Core exception vectors are wired to a common dispatch path that prints diagnostics and halts safely.
- Forced INT3 exception testing is working and used to validate the exception display path in QEMU.

## Scope boundaries

Implemented now:

- Reproducible build and run flow from source to bootable ISO.
- Stage 2 deterministic startup messages on VGA and serial.
- Minimal panic behavior with explicit halt semantics.
- IDT installation and basic exception diagnostics.

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
- kernel/init/: early kernel init and Stage 2 diagnostics.
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

### Normal Stage 2 run

VGA and COM1 serial should show:

- custom-os Stage 2: init start
- custom-os Stage 2: Multiboot2 handoff OK
- custom-os Stage 2: IDT installed
- custom-os Stage 2: deterministic init OK

### Forced INT3 exception test

Note: QEMU may appear paused after an exception. This is expected because the CPU halts (cli + hlt).

Run:

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

Expected exception-path markers:

- custom-os Stage 2: triggering INT3 test
- custom-os Stage 2 EXCEPTION
- #BP breakpoint
- vector: 0x00000003
- error : 0x00000000
- eip   : 0x????????
- cs    : 0x????????
- eflags: 0x????????

Note: eip, cs, and eflags are runtime context values and vary by run environment.

## Panic-path self-test (Stage 1 regression check)

Keep this test to ensure the Stage 1 fatal path contract remains intact while Stage 2 evolves.

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

Expected panic output (VGA and serial):

- custom-os Stage 2 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Future direction

- Timer IRQ groundwork.
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
- Stage 2 milestone document (planned as docs/milestones/stage-2.md)
