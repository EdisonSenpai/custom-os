.section .text

.global isr_stub_divide_error
.global isr_stub_breakpoint
.global isr_stub_invalid_opcode
.global isr_stub_general_protection
.global isr_stub_page_fault
.global irq_stub_timer
.global irq_stub_keyboard
.extern stage2_exception_dispatch
.extern stage3_irq0_handler
.extern stage4_irq1_handler

isr_stub_divide_error:
    push $0
    push $0
    jmp isr_common

isr_stub_breakpoint:
    push $0
    push $3
    jmp isr_common

isr_stub_invalid_opcode:
    push $0
    push $6
    jmp isr_common

isr_stub_general_protection:
    push $13
    jmp isr_common

isr_stub_page_fault:
    push $14
    jmp isr_common

isr_common:
    mov 0(%esp), %eax
    mov 4(%esp), %ebx
    mov 8(%esp), %ecx
    mov 12(%esp), %edx
    mov 16(%esp), %esi

    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
    call stage2_exception_dispatch

.halt:
    cli
    hlt
    jmp .halt

irq_stub_timer:
    pusha
    cld
    call stage3_irq0_handler
    popa
    iret

irq_stub_keyboard:
    pusha
    cld
    call stage4_irq1_handler
    popa
    iret

