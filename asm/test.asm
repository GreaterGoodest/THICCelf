;----------------------------------------------------------
;A simple test to prove injection effectiveness
;----------------------------------------------------------

section .text
global _start

_start:
    push rbp
    mov  rbp, rsp
    push rcx
    push rdx
    push r8
    push r9

    pop  r9
    pop  r8
    pop  rdx
    pop  rcx
    mov  rsp, rbp
    pop  rbp
    push 0x401040
    ret