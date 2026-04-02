# GRUB Configuration Notes

Current status: active in Stage 7.

Current files:

- grub.cfg: menu entry that loads the kernel using the Multiboot2 path.

Current behavior:

- Boots the generated kernel image from the ISO.
- Hands control to the kernel entry path used by current bring-up stages.

Scope limits:

- Minimal single-path boot configuration.
- No multi-profile boot menu or advanced runtime options yet.
