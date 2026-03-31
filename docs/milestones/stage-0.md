# Stage 0: Repository and Planning Baseline

## Objective

Establish a reproducible build and run baseline with minimal boot-path code.

## Deliverables

- [x] Build system wired through Makefile and build/*.mk.
- [x] Minimal freestanding kernel ELF target produced.
- [x] GRUB configuration added.
- [x] Bootable ISO generation added.
- [x] QEMU run scripts wired for shell and PowerShell.
- [x] Boot and linker assumptions documented.
- [x] Architecture and milestone docs updated to match implementation.

## Exit criteria

- make iso produces out/custom-os.iso.
- make run boots GRUB and reaches Stage 0 kernel marker.
- Documentation reflects current entry ABI and linker assumptions.
- Scope remains limited to build and boot pipeline only.

## Deferred to Stage 1

- Long-mode transition for x86_64 runtime.
- Multiboot2 tag parsing and validation policy.
- Panic path and serial-first logging improvements.
