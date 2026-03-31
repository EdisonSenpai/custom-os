#!/usr/bin/env sh

set -eu

ISO_PATH="${1:-out/custom-os.iso}"
QEMU_BIN="${QEMU_BIN:-qemu-system-x86_64}"

if [ ! -f "${ISO_PATH}" ]; then
	echo "ISO not found: ${ISO_PATH}" >&2
	echo "Run 'make iso' first." >&2
	exit 1
fi

exec "${QEMU_BIN}" \
	-cdrom "${ISO_PATH}" \
	-m 256M \
	-serial stdio \
	-no-reboot \
	-no-shutdown