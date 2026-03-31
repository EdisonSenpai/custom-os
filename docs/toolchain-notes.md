# Toolchain Notes (Stage 0)

Purpose: keep toolchain assumptions explicit and stable.

## Host environment

- Primary host: Windows.
- Secondary shell option: WSL.
- Emulation target: QEMU.

## Planned cross-toolchain

- Target triple: x86_64-elf.
- Compiler: GCC (freestanding).
- Binutils: assembler, linker, object inspection tools.
- Build driver: Make (placeholder in Stage 0).

## Required tools checklist

- [ ] x86_64-elf-gcc
- [ ] x86_64-elf-ld
- [ ] x86_64-elf-as
- [ ] x86_64-elf-objdump
- [ ] x86_64-elf-readelf
- [ ] grub-mkrescue or equivalent ISO tooling
- [ ] qemu-system-x86_64

## Version and path notes

- Record exact tool versions here once installed.
- Record installation source and path strategy.
- Keep build scripts independent from global machine state where possible.

## Stage 0 output

- This document is planning-only until the first build attempt is wired.