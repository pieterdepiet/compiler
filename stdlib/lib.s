.data
_printi_format: .asciz "%d\n"
.text
.extern _printf
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
_main:
    push %rbp
    movq %rsp, %rbp
    callq _start
    pop %rbp
    ret