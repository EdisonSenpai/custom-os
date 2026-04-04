# Linker Notes

Current status: Stage 10 complete and still using the same linker script baseline.

Current files:

- stage0.ld: linker script used for current early bring-up kernel image.

Current responsibilities:

- Define entry symbol and section placement for boot-time handoff.
- Keep the Multiboot2 header first via `KEEP(*(.multiboot))` and keep kernel sections laid out consistently for GRUB loading.
- Export `__kernel_phys_start` and `__kernel_phys_end` from `stage0.ld`.
- Use these bounds in early PMM reservation filtering so kernel-occupied physical memory is not handed out as allocatable frames.

Scope limits:

- Current script targets early protected-mode bring-up.
- Full x86_64 long-mode image layout work is deferred to later stages.
