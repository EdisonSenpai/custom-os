# Boot Notes

Current status: Stage 8 uses the same minimal GRUB plus Multiboot2 boot path.

Current responsibilities:

- Provide the boot configuration used to load the kernel image.
- Keep the handoff path deterministic for early kernel bring-up.

Current layout:

- grub/: GRUB menu and boot entry configuration.

Not implemented yet:

- Custom bootloader stages.
- Alternative firmware boot flows.
- Advanced boot configuration logic.
