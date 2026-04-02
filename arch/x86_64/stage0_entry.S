.section .multiboot, "a"
.align 8
mb2_header_start:
.long 0xE85250D6
.long 0
.long mb2_header_end - mb2_header_start
.long -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start))

.short 0
.short 0
.long 8
mb2_header_end:

.section .text
.global stage0_start
.extern stage0_main

stage0_start:
    cli
    mov $stack_top, %esp

    # Multiboot2 entry regs:
    # eax = magic, ebx = multiboot info pointer.
    push %ebx
    push %eax
    call stage0_main

.hang:
    sti
    hlt
    jmp .hang

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:
