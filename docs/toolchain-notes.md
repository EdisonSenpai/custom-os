# Toolchain Notes (Stage 0)

Purpose: keep Stage 0 build assumptions explicit and reproducible.

## Host environment

- Primary host: Windows.
- Preferred build shell: WSL or Linux shell.
- Secondary shell option: PowerShell for run-only flow.
- Emulator target: qemu-system-x86_64.

## Planned cross-toolchain

- Stage 0 entry ABI: i386 (Multiboot2 protected-mode entry).
- Long-term architecture target: x86_64.
- Recommended Stage 0 triple: i686-elf.
- Compiler: i686-elf-gcc (freestanding).
- Binutils: i686-elf-as, i686-elf-ld, i686-elf-readelf, i686-elf-objdump.
- Build driver: GNU Make.

## Required tools checklist

- [ ] i686-elf-gcc
- [ ] i686-elf-ld
- [ ] i686-elf-as
- [ ] i686-elf-objdump
- [ ] i686-elf-readelf
- [ ] grub-mkrescue
- [ ] xorriso (usually required by grub-mkrescue)
- [ ] qemu-system-x86_64
- [ ] make

## Version and path notes

- Record exact tool versions here once installed.
- Record installation source and path strategy.
- Keep build scripts independent from global machine state where possible.

Suggested first capture:
- i686-elf-gcc --version
- i686-elf-ld --version
- grub-mkrescue --version
- qemu-system-x86_64 --version

## Stage 0 command baseline

- make help
- make kernel
- make iso
- make inspect
- make run
