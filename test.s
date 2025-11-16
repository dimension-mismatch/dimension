#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
  mov qword ptr [rsp - 8], 11
  mov rax, 3
  add rax, 7
  mov rbx, qword ptr [rsp - 8]
  add rbx, 5

  add rax, rbx
  mov qword ptr [rsp - 16], rax
  mov rcx, qword ptr [rsp - 16]
  mov rdi, rcx
  mov rax, 0x2000001
  syscall

