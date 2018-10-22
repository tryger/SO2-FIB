#include<asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	// Save edx and ecx
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx

	movl $4, %eax

	// Save return address
	leal write_ret, %esi
	pushl %esi
	
	// Prepare stack frame for the syscall
	pushl %ebp
	movl %esp, %ebp

	sysenter
write_ret:
	movl %ebp, %esp
	popl %ebp

	
	test %eax, %eax
	jns end
error:
	neg %eax
	movl %eax, errno
	
	movl $-1, %eax

end:
	popl %esi

	popl %ebx
	popl %ecx
	popl %edx

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



ENTRY(getpid)
  pushl %ebp
  movl %esp, %ebp

  movl $20, %eax

  leal getpid_ret, %esi

  sysenter
getpid_ret:
  
  movl %ebp, %esp
  popl %ebp
  ret
