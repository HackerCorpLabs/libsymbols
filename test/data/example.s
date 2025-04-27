	.file	"example.c"
	.stabs	"example.c",100,0,2,.Ltext0
	.text
.Ltext0:
	.stabs	"gcc2_compiled.",60,0,0,0
	.globl	global_var
	.data
	.align 4
	.type	global_var, @object
	.size	global_var, 4
global_var:
	.long	42
	.stabs	"global_var:G(0,1)=r(0,1);-2147483648;2147483647;",32,0,0,0
	.stabs	"int:t(0,1)",128,0,0,0
	.align 4
	.type	static_var, @object
	.size	static_var, 4
static_var:
	.long	24
	.stabs	"static_var:S(0,1)",38,0,0,static_var
	.section	.rodata
.LC0:
	.string	"Hello, World!"
	.text
	.stabs	"main:F(0,1)",36,0,0,main
	.globl	main
	.type	main, @function
main:
	.stabn	68,0,12,.LM0-.LFBB1
.LM0:
.LFBB1:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	.stabn	68,0,13,.LM1-.LFBB1
.LM1:
	movl	$10, -8(%rbp)
	.stabn	68,0,14,.LM2-.LFBB1
.LM2:
	movl	global_var(%rip), %edx
	movl	-8(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	add_numbers
	movl	%eax, -4(%rbp)
	.stabn	68,0,15,.LM3-.LFBB1
.LM3:
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	print_message
	.stabn	68,0,16,.LM4-.LFBB1
.LM4:
	movl	-4(%rbp), %eax
	.stabn	68,0,17,.LM5-.LFBB1
.LM5:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.stabs	"local_var:(0,1)",128,0,0,-8
	.stabs	"result:(0,1)",128,0,0,-4
	.stabn	192,0,0,.LFBB1-.LFBB1
	.stabn	224,0,0,.Lscope1-.LFBB1
.Lscope1:
	.stabs	"add_numbers:F(0,1)",36,0,0,add_numbers
	.stabs	"a:p(0,1)",160,0,0,-4
	.stabs	"b:p(0,1)",160,0,0,-8
	.globl	add_numbers
	.type	add_numbers, @function
add_numbers:
	.stabn	68,0,20,.LM6-.LFBB2
.LM6:
.LFBB2:
.LFB1:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	.stabn	68,0,21,.LM7-.LFBB2
.LM7:
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	addl	%edx, %eax
	.stabn	68,0,22,.LM8-.LFBB2
.LM8:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	add_numbers, .-add_numbers
.Lscope2:
	.stabs	"print_message:F(0,2)=(0,2)",36,0,0,print_message
	.stabs	"void:t(0,2)",128,0,0,0
	.stabs	"msg:p(0,3)=*(0,4)=r(0,4);0;127;",160,0,0,-8
	.stabs	"char:t(0,4)",128,0,0,0
	.globl	print_message
	.type	print_message, @function
print_message:
	.stabn	68,0,24,.LM9-.LFBB3
.LM9:
.LFBB3:
.LFB2:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.stabn	68,0,25,.LM10-.LFBB3
.LM10:
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	.stabn	68,0,26,.LM11-.LFBB3
.LM11:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	print_message, .-print_message
.Lscope3:
	.stabs	"",100,0,0,.Letext0
.Letext0:
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
