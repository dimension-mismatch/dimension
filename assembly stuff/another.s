.global start
.intel_syntax noprefix


start:
  push rbp
  mov rbp, rsp
  sub rsp, 4

  mov qword ptr [rsp - 8], 3
  mov qword ptr [rsp - 16], 20

  mov rdi, qword ptr [rsp - 16]
  add rdi, qword ptr [rsp - 8]
  mov rax, 0x2000001
  syscall

