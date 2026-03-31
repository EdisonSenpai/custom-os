param(
    [string]$IsoPath = "out\\custom-os.iso"
)

$qemu = if ($env:QEMU_BIN) { $env:QEMU_BIN } else { "qemu-system-x86_64" }

if (-not (Test-Path -Path $IsoPath)) {
    Write-Error "ISO not found: $IsoPath"
    Write-Host "Run 'make iso' first."
    exit 1
}

& $qemu `
    -cdrom $IsoPath `
    -m 256M `
    -serial stdio `
    -no-reboot `
    -no-shutdown

exit $LASTEXITCODE
