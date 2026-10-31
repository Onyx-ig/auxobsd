.section .text
.global _start

_start:
    lea msg(%rip), %rsi
    mov $1, %rax
    mov $1, %rdi
    mov $15, %rdx
    syscall

    mov $60, %rax
    xor %rdi, %rdi
    syscall

.section .rodata
msg:
    .ascii "Idk just here\n"

.section .alias, "a"
alias_msg:
    .ascii "Meow"
