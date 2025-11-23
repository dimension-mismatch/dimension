#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, 3


  imul rax, 2
#1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rbx, 5


  add rbx, rax

  sub rsp, 40
  mov qword ptr [rbp - 16], rbx
  mov qword ptr [rbp - 24], 5
  mov qword ptr [rbp - 32], 3
  lea rax, [rbp - 8]
  mov qword ptr [rbp - 40], rax
call function_body_4
  mov rbx, qword ptr [rbp - 16]
  add rsp, 32

  mov rdi, qword ptr [rbp - 8]
  mov rax, 0x2000001
  syscall

function_body_4:
  push rbp
  mov rbp, rsp
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp + 8]


  sub rax, qword ptr [rbp + 8]
  mov rbx, qword ptr [rbp + 16]
  mov [rbx], rax
  pop rbp 
  ret

  pop rbp 
  ret
