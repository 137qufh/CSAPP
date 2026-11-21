# int add_loop(int n)
# Sum 1 + 2 + ... + n. If n <= 0, return 0.
#
# System V AMD64: the argument arrives in %edi, the int result leaves
# in %eax. %ecx is caller-saved, so this leaf does not touch the stack.

        .text
        .p2align 4
        .globl  add_loop
        .type   add_loop, @function
add_loop:
        xorl    %eax, %eax
        testl   %edi, %edi
        jle     .Ldone
        movl    $1, %ecx
.Lloop:
        addl    %ecx, %eax
        addl    $1, %ecx
        cmpl    %edi, %ecx
        jle     .Lloop
.Ldone:
        ret
        .size   add_loop, .-add_loop

        .section .note.GNU-stack, "", @progbits
