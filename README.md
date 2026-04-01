# CustomOS (working title: AnimeOS)

> Current status: Stage 6 complete (6A + 6B + 6C + 6D) and verified in QEMU

CustomOS is a from-scratch operating system project focused on low-level correctness, inspectability, and disciplined incremental bring-up. The current baseline is Stage 6 complete, combining Stage 4 interrupt behavior with a validated minimal PMM lifecycle.

## Architecture snapshot (Stage 6)

- 32-bit protected mode (Multiboot2 entry)
- No paging yet
- Bootloader-provided GDT in use
- IDT installed at runtime
- Exceptions handled via assembly stubs to C dispatcher
- PIC remapped for hardware IRQ vectors
- PIT configured for periodic IRQ0 timer interrupts
- Keyboard IRQ1 routed through IDT and raw scancodes read from port 0x60
- Multiboot2 memory map parsing and early frame bookkeeping
- Minimal PMM API: deterministic frame allocation, minimal free API, deterministic pending-free tracking, and FIFO reuse activation

## Current baseline (Stage 6)

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
- PIC remap and IRQ0/IRQ1 operation with EOI handling.
- PIT periodic tick configuration.
- Recurring Stage 6 tick output on COM1 serial while idle.
- Raw keyboard scancode logging on COM1 serial.
- Stage 5A: Multiboot2 memory map parsing.
- Stage 5B: raw and policy-usable RAM accounting summary.
- Stage 5C: frame-range bookkeeping groundwork.
- Stage 5D: deterministic early-boot frame allocation smoke test.
- Stage 6A: PMM state formalization and internal cleanup.
- Stage 6B: minimal exported PMM allocation API.
- Stage 6C: minimal exported PMM free API and deterministic pending-free tracking.
- Stage 6D: deterministic FIFO reuse activation and dedicated reuse self-test validation.

## Stage 6 highlights

- Stage 4 interrupt runtime behavior remains active after Stage 6 PMM lifecycle bring-up.
- Stage 5A and 5B provide safe parsing and accounting summaries from Multiboot2 data.
- Stage 5C establishes allocator-ready frame range bookkeeping.
- Stage 5D proves deterministic frame handout behavior without introducing full allocator lifecycle complexity.
- Stage 6C adds deterministic pending-free tracking.
- Stage 6D activates deterministic FIFO reuse while preserving baseline boot/runtime behavior.

## Scope boundaries

Implemented now:

- Reproducible build and run flow from source to bootable ISO.
- Stage 6 deterministic startup messages on VGA and serial.
- Stage 4 IRQ runtime behavior (timer plus keyboard) and Stage 2 exception diagnostics.
- Minimal PMM lifecycle through Stage 6A to Stage 6D.

Deliberately not implemented yet:

- Advanced frame lifecycle features beyond current deterministic minimal PMM model.
- Paging and virtual memory.
- Heap/kmalloc.
- Scheduler and task switching.
- Filesystem and user-mode runtime.
- Full x86_64 long-mode runtime.

## Repository map

- docs/: architecture decisions, boot assumptions, milestones, and toolchain notes.
- boot/grub/: GRUB configuration for bootable image generation.
- arch/x86_64/: architecture-specific entry and interrupt/exception assembly stubs.
- linker/: linker script and memory layout contract.
- kernel/init/: early kernel init and Stage 6 diagnostics.
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

### Normal Stage 6 run

VGA and COM1 serial should show:

- custom-os Stage 6: init start
- custom-os Stage 5A/B/C/D summary markers
- custom-os Stage 6: Multiboot2 handoff OK
- custom-os Stage 6: IDT installed
- custom-os Stage 6: PIC remapped + PIT started
- custom-os Stage 6: deterministic init OK
- recurring serial lines like custom-os Stage 6 tick: 0x00000064
- keyboard serial lines like custom-os Stage 6 scancode: 0x0000001E

### Stage 6D dedicated reuse self-test

Note: The current build system does not yet wire that flag through and the temporary source define was used for validation. The self-test is enabled temporarily by setting STAGE6D_FORCE_REUSE_TEST in kernel/init/stage0_main.c:

```sh
make clean
make run STAGE6D_FORCE_REUSE_TEST=1
```

Expected serial markers include:

- custom-os Stage 6D test alloc A/B: PASS
- custom-os Stage 6D test free A accept: PASS
- custom-os Stage 6D test free B accept: PASS
- custom-os Stage 6D test duplicate free reject: PASS
- custom-os Stage 6D test never-issued reject: PASS
- custom-os Stage 6D test pending count after free: PASS
- custom-os Stage 6D test fifo reuse order: PASS
- custom-os Stage 6D test pending drained: PASS
- custom-os Stage 6D test overall: PASS

### Forced INT3 exception test

Note: QEMU may appear paused after an exception. This is expected because the CPU halts (cli + hlt).

```sh
make clean
make run STAGE2_FORCE_EXCEPTION=1
```

Expected exception-path markers:

- custom-os Stage 6: triggering INT3 test
- custom-os Stage 2 EXCEPTION
- #BP breakpoint
- vector: 0x00000003
- error : 0x00000000
- eip   : 0x????????
- cs    : 0x????????
- eflags: 0x????????

## Panic-path self-test (Stage 1 regression check)

Keep this test available as a regression guard.

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

Expected panic output (VGA and serial):

- custom-os Stage 6 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Future direction

- Stage 7 planning only.
- Paging groundwork definition and validation gates.
- Keep current Stage 6 baseline stable while Stage 7 scope is finalized.

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
- docs/milestones/stage-5a.md
- docs/milestones/stage-5b.md
- docs/milestones/stage-5c.md
- docs/milestones/stage-5d.md
- docs/milestones/stage-6a.md
- docs/milestones/stage-6b.md
- docs/milestones/stage-6c.md
- docs/milestones/stage-6d.md
