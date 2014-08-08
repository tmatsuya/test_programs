# Assembler TANKA sample program (5-7-5-7-7 -> 31 bytes)
# Program abstract: character base movie player
# Environment: Linux 64bit only
# How to compile:
#	as --64 -o player575.o player575.s 
#	ld -s -o player575 player575.o
# How to use:
#	./player575 </dev/tcp/macchan.sfc.wide.ad.jp/8081
#	 or
#	./player575 </dev/tcp/macchan.sfc.wide.ad.jp/8082
#
.code64
.text
.global _start
_start:
	# 5 byte
	cld
loop:
	xorl %eax,%eax  # sys_read
	push %rax
	pop %rdi	# stdin

	# 7 byte
	movb $1, %dl    # length
	movl $msg, %esi

	# 5 byte
	syscall
	andb %al, %al
	cmc

	# 7 byte
	jz exit		# EOF? else eax = 1(sys_write and stdout)
	decb (%esi)	# decode
	movl %eax, %edi # stdout

	# 7byte
	syscall
	jmp loop
exit:	
	clc
	int3		# eax=0 exit by breakpoint trap
	hlt

.bss
	msg:	.skip 1
