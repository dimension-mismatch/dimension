#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
  sub rsp, 88
  mov qword ptr [rbp - 80], 8

  mov qword ptr [rbp - 88], 18

  sub rsp, 24
  mov rax, qword ptr [rbp - 88]
  mov qword ptr [rbp - 104], rax
  mov rax, qword ptr [rbp - 80]
  mov qword ptr [rbp - 112], rax
call function_body_5


  mov rdi, qword ptr [rbp - 88]
  mov rax, 0x2000001
  syscall

function_body_5:
  push rbp
  mov rbp, rsp
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp + 16]


  add rax, qword ptr [rbp + 24]

  mov rdi, rax
  mov rax, 0x2000001
  syscall

