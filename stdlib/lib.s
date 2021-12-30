.data
_printi_format: .asciz "%d\n"
_printp_format: .asciz "%p\n"
_prints_format: .asciz "%s\n"
.text
.extern _printf
.extern _malloc
.globl _main
.globl _printi
_printi:
    push %rbp
    movq %rsp, %rbp
    movl %edi, %esi
    movq _printi_format@GOTPCREL(%rip), %rdi
    callq _printf
    xorl %eax, %eax
    pop %rbp
    ret
_printp:
    push %rbp
    movq %rsp, %rbp
    movl %edi, %esi
    movq _printp_format@GOTPCREL(%rip), %rdi
    callq _printf
    xorl %eax, %eax
    pop %rbp
    ret
.globl _prints
_prints:
    push %rbp
    movq %rsp, %rbp
    movq %rdi, %rsi
    leaq _prints_format(%rip), %rdi
    callq _printf
    pop %rbp
    ret
.globl _C3Int5print
_C3Int5print:
    push %rbp
    movq %rsp, %rbp
    movl %edi, %esi
    movl %esi, %eax
    movq _printi_format@GOTPCREL(%rip), %rdi
    callq _printf
    pop %rbp
    ret
.globl _C3Int3add
_C3Int3add:
    movl %edi, %eax
    addl %esi, %eax
    ret
_main:
    push %rbp
    movq %rsp, %rbp
    callq _start
    pop %rbp
    ret