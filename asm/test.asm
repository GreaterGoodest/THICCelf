;----------------------------------------------------------
;A simple test to prove injection effectiveness
;----------------------------------------------------------

section .text
global _start

_start:
    ;save registers 
    push rbp
    mov  rbp, rsp
    push rcx
    push rdx
    push r8
    push r9
    push rax
    push rbx

    ;set up write syscall
    mov  rdi, 1 ;stdout
    push 0x0a6e7770 ;pwn\n
    mov  rsi, rsp
    mov  rdx, 0x4
    mov  rax, 0x1 ;write syscall number
    syscall

    ;restore registers
    pop  rbx ;clear string from stack
    xor  rbx, rbx
    pop  rbx
    pop  rax
    pop  r9
    pop  r8
    pop  rdx
    pop  rcx
    mov  rsp, rbp
    pop  rbp
    push 0x401070
    ret