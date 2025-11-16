.global start
.intel_syntax noprefix


start:
  push rbp
  mov rbp, rsp
  sub rsp, 4

  mov qword ptr [rsp - 4], 3
  mov qword ptr [rsp - 8], 20

  mov rdi, 0
  add rdi, qword ptr [rsp - 4]
  mov rax, 0x2000001
  syscall

