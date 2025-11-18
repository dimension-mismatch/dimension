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
	subq	$16, %rsp
	movl	$0, -4(%rbp)
	movl	$8, -8(%rbp)
	movl	-8(%rbp), %eax
	addl	$15, %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %edi
	callq	_exit
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
