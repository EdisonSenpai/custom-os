# Stage 0 placeholder build configuration

TARGET_ARCH := x86_64
TOOLCHAIN_PREFIX := x86_64-elf

# Replace with real output layout in Stage 1.
OUT_DIR := out
KERNEL_ELF := $(OUT_DIR)/kernel.elf
BOOT_ISO := $(OUT_DIR)/custom-os.iso