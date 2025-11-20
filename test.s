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


  sub rbx, rax
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, 3


  add rax, rbx
  mov qword ptr [rbp - 88], rax

  mov rax, qword ptr [rbp - 88]
  mov qword ptr [rbp + 16], rax
  mov rax, qword ptr [rbp - 80]
  mov qword ptr [rbp + 16], rax
call function_body_5


  mov rdi, qword ptr [rbp - 88]
  mov rax, 0x2000001
  syscall

function_body_5:
  push rbp
  mov rbp, rsp
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp + 16]


  sub rax, 3

  imul rax, qword ptr [rbp + 8]
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rbx, qword ptr [rbp + 8]


  add rbx, rax

  mov qword ptr [rbp + 3], rbx

