# Stage 0 build targets

.PHONY: all help check-tools kernel iso run inspect clean

all: iso

help:
	@echo "Targets:"
	@echo "  make all      - Build bootable ISO"
	@echo "  make kernel   - Build kernel ELF only"
	@echo "  make iso      - Build GRUB bootable ISO"
	@echo "  make run      - Build ISO and run in QEMU"
	@echo "  make inspect  - Inspect ELF headers and symbols"
	@echo "  make clean    - Remove build artifacts"

check-tools:
	@command -v $(CC) >/dev/null || (echo "Missing tool: $(CC)" && exit 1)
	@command -v $(LD) >/dev/null || (echo "Missing tool: $(LD)" && exit 1)
	@command -v $(AS) >/dev/null || (echo "Missing tool: $(AS)" && exit 1)
	@command -v $(GRUB_MKRESCUE) >/dev/null || (echo "Missing tool: $(GRUB_MKRESCUE)" && exit 1)
	@command -v $(QEMU_BIN) >/dev/null || (echo "Missing tool: $(QEMU_BIN)" && exit 1)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

$(ENTRY_OBJ): $(ENTRY_SRC) | $(OUT_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(EXCEPTION_STUB_OBJ): $(EXCEPTION_STUB_SRC) | $(OUT_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(KERNEL_OBJ): $(KERNEL_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(PAGING_OBJ): $(PAGING_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(VMM_LAYOUT_OBJ): $(VMM_LAYOUT_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(VMM_OBJ): $(VMM_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KHEAP_OBJ): $(KHEAP_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KMALLOC_OBJ): $(KMALLOC_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KMEMORY_OBJ): $(KMEMORY_SRC) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(ENTRY_OBJ) $(EXCEPTION_STUB_OBJ) $(KERNEL_OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(ENTRY_OBJ) $(EXCEPTION_STUB_OBJ) $(KERNEL_OBJS)

kernel: check-tools $(KERNEL_ELF)
	@echo "Built $(KERNEL_ELF)"

$(BOOT_ISO): $(KERNEL_ELF) $(GRUB_CFG)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	@cp $(GRUB_CFG) $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $@ $(ISO_DIR)

iso: check-tools $(BOOT_ISO)
	@echo "Built $(BOOT_ISO)"

run: iso
	@sh scripts/run-qemu.sh $(BOOT_ISO)

inspect: kernel
	@echo "ELF header:"
	@$(READELF) -h $(KERNEL_ELF)
	@echo "Entry symbol check:"
	@$(READELF) -s $(KERNEL_ELF) | grep stage0_start || true

clean:
	rm -rf $(OUT_DIR)
	@echo "Removed $(OUT_DIR)"
