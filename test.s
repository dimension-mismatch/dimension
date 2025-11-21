#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov qword ptr [rbp - 8], 8

  mov qword ptr [rbp - 16], 18

  sub rsp, 32
  mov rax, qword ptr [rbp - 16]
  mov qword ptr [rbp - 40], rax
  mov rax, qword ptr [rbp - 8]
  mov qword ptr [rbp - 48], rax
call function_body_5
  add rsp, 24


  mov rdi, qword ptr [rbp - 16]
  mov rax, 0x2000001
  syscall

function_body_5:
  push rbp
  mov rbp, rsp
 #1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp + 16]


  add rax, qword ptr [rbp + 24]

  pop rbp 
  ret
