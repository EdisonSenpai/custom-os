# Boot Notes

Current status: Stage 11 complete (11A through 11E) and still using the same minimal GRUB plus Multiboot2 boot path.

Current responsibilities:

- Provide the boot configuration used to load the kernel image.
- Keep the handoff path deterministic for early kernel bring-up.

Current layout:

- grub/: GRUB menu and boot entry configuration.

Current GRUB baseline:

- GRUB menu entry uses `multiboot2 /boot/kernel.elf`.
- Menu entry text is `custom-os` and no longer refers to Stage 0.

Not implemented yet:

- Custom bootloader stages.
- Alternative firmware boot flows.
- Advanced boot configuration logic.
