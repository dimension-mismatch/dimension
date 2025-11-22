#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
  sub rsp, 32
  mov qword ptr [rbp - 16], 5
  mov qword ptr [rbp - 24], 3
  lea rax, [rbp - 8]
  mov qword ptr [rbp - 32], rax
call function_body_4
  add rsp, 24

  mov rdi, qword ptr [rbp - 8]
  mov rax, 0x2000001
  syscall

function_body_4:
  push rbp
  mov rbp, rsp
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov rax, qword ptr [rbp + 24]


  add rax, qword ptr [rbp + 32]
  mov rbx, qword ptr [rbp + 16]
  mov [rbx], rax
  pop rbp 
  ret

  pop rbp 
  ret
