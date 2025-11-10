global _main:
section .text
_main:
  mov rax, 0x2000004
  mov rdi, 1
  lea rsi, [rel str]
  mov rdx, str.len
  syscall

  mov rax, 0x2000001
  mov rdi, 69
  syscall

section .data
str:  db "Hello World"
.len  equ  $ - str