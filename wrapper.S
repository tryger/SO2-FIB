#include<asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx

	movl $4, %eax

	pushl %edx
	pushl %ecx

	leal write_ret, %esi
	pushl %esi
	
	pushl %ebp
	movl %esp, %ebp

	sysenter
write_ret:
	movl %ebp, %esp
	popl %ebp

	popl %ecx
	popl %edx
	

	// error check

	

	movl %ebp, %esp
	popl %ebp

	ret


ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	movl $10, %eax

	pushl %edx
	pushl %ecx

	leal gettime_ret, %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
gettime_ret:

	// error check

	movl %ebp, %esp
	popl %ebp

	ret
