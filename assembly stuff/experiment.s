	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 14, 5
	.globl	_three_x_plus_one               ## -- Begin function three_x_plus_one
	.p2align	4, 0x90
_three_x_plus_one:                      ## @three_x_plus_one
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	%edi, -4(%rbp)
	imull	$3, -4(%rbp), %eax
	addl	$1, %eax
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.globl	_main                           ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$32, %rsp
	movl	$5, -4(%rbp)
	movl	$6, -8(%rbp)
	movl	-4(%rbp), %eax
	addl	-8(%rbp), %eax
	movl	%eax, -16(%rbp)                 ## 4-byte Spill
	movl	-4(%rbp), %eax
	movl	%eax, -20(%rbp)                 ## 4-byte Spill
	movl	-8(%rbp), %edi
	callq	_three_x_plus_one
	movl	-20(%rbp), %ecx                 ## 4-byte Reload
	movl	%eax, %edx
	movl	-16(%rbp), %eax                 ## 4-byte Reload
	addl	%edx, %ecx
	addl	%ecx, %eax
	movl	%eax, -12(%rbp)
	xorl	%eax, %eax
	addq	$32, %rsp
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
