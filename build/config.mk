# Stage 0 build configuration

# Long-term project target remains x86_64. Stage 0 uses a Multiboot2
# protected-mode entry path to keep boot pipeline bring-up minimal.
TARGET_ARCH := x86_64
BOOT_ENTRY_MODE := i386

TOOLCHAIN_PREFIX ?= i686-elf
STAGE1_FORCE_PANIC ?= 0
STAGE2_FORCE_EXCEPTION ?= 0

CC := $(TOOLCHAIN_PREFIX)-gcc
LD := $(TOOLCHAIN_PREFIX)-ld
AS := $(TOOLCHAIN_PREFIX)-as
OBJDUMP := $(TOOLCHAIN_PREFIX)-objdump
READELF := $(TOOLCHAIN_PREFIX)-readelf

GRUB_MKRESCUE ?= grub-mkrescue
QEMU_BIN ?= qemu-system-x86_64

OUT_DIR := out
ISO_DIR := $(OUT_DIR)/iso
KERNEL_ELF := $(OUT_DIR)/kernel.elf
BOOT_ISO := $(OUT_DIR)/custom-os.iso

ENTRY_SRC := arch/x86_64/stage0_entry.S
EXCEPTION_STUB_SRC := arch/x86_64/stage2_exceptions.S
KERNEL_SRC := kernel/init/stage0_main.c
PAGING_SRC := kernel/mm/paging.c
VMM_LAYOUT_SRC := kernel/mm/vmm_layout.c
KERNEL_INCLUDE_DIR := kernel/include
LINKER_SCRIPT := linker/stage0.ld
GRUB_CFG := boot/grub/grub.cfg

ENTRY_OBJ := $(OUT_DIR)/stage0_entry.o
EXCEPTION_STUB_OBJ := $(OUT_DIR)/stage2_exceptions.o
KERNEL_OBJ := $(OUT_DIR)/stage0_main.o
PAGING_OBJ := $(OUT_DIR)/paging.o
VMM_LAYOUT_OBJ := $(OUT_DIR)/vmm_layout.o
KERNEL_OBJS := $(KERNEL_OBJ) $(PAGING_OBJ) $(VMM_LAYOUT_OBJ)

CFLAGS := -std=c11 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -Wall -Wextra -Werror -m32 -I$(KERNEL_INCLUDE_DIR) -DSTAGE1_FORCE_PANIC=$(STAGE1_FORCE_PANIC) -DSTAGE2_FORCE_EXCEPTION=$(STAGE2_FORCE_EXCEPTION)
ASFLAGS := --32
LDFLAGS := -m elf_i386 -T $(LINKER_SCRIPT) -nostdlib

QEMU_FLAGS := -cdrom $(BOOT_ISO) -m 256M -serial stdio -no-reboot -no-shutdown
