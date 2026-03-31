# custom-os

From-scratch operating system project with a disciplined Stage 0 boot pipeline baseline.

## Current baseline (Stage 0)

- Build pipeline via Make.
- Minimal freestanding kernel ELF target.
- GRUB + Multiboot2 bootable ISO generation.
- QEMU run scripts for WSL/Linux and Windows PowerShell.
- Boot and linker assumptions documented before deeper kernel work.

## Scope boundaries

Implemented in this stage:
- Reproducible build and run path.
- Boot artifact generation and inspection entry points.
- Minimal visible boot marker in QEMU.

Deliberately not implemented yet:
- Scheduler.
- Memory manager.
- Filesystem.
- Drivers.
- Full x86_64 long-mode runtime.

## Repository map

- docs/: architecture decisions, milestone tracking, toolchain notes, and handoff assumptions.
- boot/grub/: GRUB menu configuration.
- arch/x86_64/: architecture entry source for Stage 0.
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
- QEMU boots GRUB and then transfers to the Stage 0 kernel.
- Screen shows:
	- custom-os Stage 0: boot pipeline baseline OK

## Key docs

- docs/architecture-decisions.md
- docs/boot-handoff-notes.md
- docs/toolchain-notes.md
- docs/milestones/stage-0.md
- docs/milestone-tracker.md
