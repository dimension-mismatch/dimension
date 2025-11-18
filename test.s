#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp

  sub rsp, 88
  mov qword ptr [rbp - 80], 8

#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp - 80]


  add rax, 5
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rbx, 7


  imul rbx, rax
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, 3


  add rax, rbx
  mov qword ptr [rbp - 88], rax


  mov rdi, qword ptr [rbp - 88]
  mov rax, 0x2000001
  syscall

function_body_4:
  push rbp
  mov rbp, rsp
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rbx, qword ptr [rbp + 8]


  add rbx, qword ptr [rbp + 16]
#1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rcx, 3


  imul rcx, qword ptr [rbp + 8]

  sub rbx, rcx

