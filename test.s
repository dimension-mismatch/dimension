#DIMENSION v0.0.1 compiled
.global start
.intel_syntax noprefix

start:
  push rbp
  mov rbp, rsp
  sub rsp, 2
  mov byte ptr [rbp - 1], 6

  sub rsp, 9
  mov al, byte ptr [rbp - 1]
  mov byte ptr [rbp - 3], al
call function_body_7
  add rsp, 9

  mov byte ptr [rbp - 1], 7

  sub rsp, 9
  mov al, byte ptr [rbp - 1]
  mov byte ptr [rbp - 3], al
call function_body_7
  add rsp, 9

  mov byte ptr [rbp - 2], 72


  mov rdi, 1
  lea rsi, byte ptr [rbp - 2]
  mov rdx, 1
  mov rax, 0x2000004
  syscall

  mov byte ptr [rbp - 2], 101


  mov rdi, 1
  lea rsi, byte ptr [rbp - 2]
  mov rdx, 1
  mov rax, 0x2000004
  syscall

  mov byte ptr [rbp - 2], 108


  mov rdi, 1
  lea rsi, byte ptr [rbp - 2]
  mov rdx, 1
  mov rax, 0x2000004
  syscall


  mov rdi, 1
  lea rsi, byte ptr [rbp - 2]
  mov rdx, 1
  mov rax, 0x2000004
  syscall

  mov byte ptr [rbp - 2], 111


  mov rdi, 1
  lea rsi, byte ptr [rbp - 2]
  mov rdx, 1
  mov rax, 0x2000004
  syscall


  mov rdi, 0
  mov rax, 0x2000001
  syscall

  add rsp, 2
function_body_6:
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
function_body_7:
  push rbp
  mov rbp, rsp
  sub rsp, 1
#1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  mov al, byte ptr [rbp + 24]


  add al, 48
  mov byte ptr [rbp - 1], al


  mov rdi, 1
  lea rsi, byte ptr [rbp - 1]
  mov rdx, 1
  mov rax, 0x2000004
  syscall

  add rsp, 1
  pop rbp 
  ret
