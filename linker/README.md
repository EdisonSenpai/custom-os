# Linker Notes

Current status: Stage 8 still uses the same linker script baseline.

Current files:

- stage0.ld: linker script used for current early bring-up kernel image.

Current responsibilities:

- Define entry symbol and section placement for boot-time handoff.
- Keep multiboot header and kernel sections laid out consistently for GRUB loading.
- Export `__kernel_phys_start` and `__kernel_phys_end` from `stage0.ld` so early PMM policy can reserve kernel-occupied physical memory from allocation.

Scope limits:

- Current script targets early protected-mode bring-up.
- Full x86_64 long-mode image layout work is deferred to later stages.
