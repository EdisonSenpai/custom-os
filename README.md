# CustomOS [TBD]

From-scratch operating system project with a disciplined Stage 1 deterministic init baseline.

## Current baseline (Stage 1)

- Build pipeline via Make.
- Minimal freestanding kernel ELF target.
- GRUB + Multiboot2 bootable ISO generation.
- QEMU run scripts for WSL/Linux and Windows PowerShell.
- Explicit early init ordering in kernel entry.
- Multiboot2 entry magic validation.
- Minimal panic path for fatal early boot errors.
- Boot and linker assumptions documented before deeper kernel work.

## Scope boundaries

Implemented in this stage:

- Reproducible build and run path.
- Boot artifact generation and inspection entry points.
- Deterministic Stage 1 init status output to VGA and serial.
- Panic output to VGA and serial, followed by halt.

Deliberately not implemented yet:

- Scheduler.
- Memory manager.
- Filesystem.
- Drivers.
- Full x86_64 long-mode runtime.

## Repository map

- docs/: architecture decisions, milestone tracking, toolchain notes, and handoff assumptions.
- boot/grub/: GRUB menu configuration.
- arch/x86_64/: architecture entry source for Stage 0 and Stage 1 bootstrap.
- linker/: linker scripts and layout contracts.
- kernel/init/: minimal early kernel entry in C.
- build/: shared Make configuration and targets.
- scripts/: QEMU run wrappers.

## Build commands

WSL/Linux shell:

```sh
make help
make iso
make run
```

Windows PowerShell:

```powershell
make iso
.\scripts\run-qemu.ps1 -IsoPath out\custom-os.iso
```

## Expected output

- Build creates:
  - out/kernel.elf
  - out/custom-os.iso
- QEMU boots GRUB and then transfers to the Stage 1 kernel entry path.
- Screen shows:
  - custom-os Stage 1: init start
  - custom-os Stage 1: Multiboot2 handoff OK
  - custom-os Stage 1: deterministic init OK
- Serial (COM1) shows the same Stage 1 status lines.

## Panic-path self-test

WSL/Linux shell:

```sh
make clean
make run STAGE1_FORCE_PANIC=1
```

Windows PowerShell:

```powershell
make clean
make run STAGE1_FORCE_PANIC=1
```

Expected panic output (VGA and serial):

- custom-os Stage 1 PANIC
- forced panic for Stage 1 test
- detail: 0x0000F001

## Key docs

- docs/architecture-decisions.md
- docs/boot-handoff-notes.md
- docs/toolchain-notes.md
- docs/milestones/stage-0.md
- docs/milestones/stage-1.md
- docs/milestone-tracker.md
