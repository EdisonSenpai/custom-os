# Linker Notes

Current status: Stage 6 still uses the same linker script baseline.

Current files:

- stage0.ld: linker script used for current early bring-up kernel image.

Current responsibilities:

- Define entry symbol and section placement for boot-time handoff.
- Keep multiboot header and kernel sections laid out consistently for GRUB loading.

Scope limits:

- Current script targets early protected-mode bring-up.
- Full x86_64 long-mode image layout work is deferred to later stages.
