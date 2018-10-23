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
  pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
gettime_ret:

	movl %ebp, %esp
	popl %ebp

  popl %esi

	// error check

  popl %ecx
  popl %edx

	movl %ebp, %esp
	popl %ebp
	ret



ENTRY(getpid)
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx

  movl $20, %eax
  leal getpid_ret, %esi
  pushl %esi

  pushl %ebp
  movl %esp, %ebp

  sysenter
getpid_ret:

  movl %ebp, %esp
  popl %ebp

  popl %esi

  // error check

  popl %edx
  popl %ecx
  
  movl %ebp, %esp
  popl %ebp
  ret


ENTRY(fork)
  pushl %ebp
  movl %esp, %ebp

  pushl %ecx
  pushl %edx

  movl $2, %eax
  leal fork_ret, %esi
  pushl %esi

  pushl %ebp
  movl %esp, %ebp

  sysenter
fork_ret:

  movl %ebp, %esp
  popl %ebp

  popl %esi

  test %eax, %eax
  jns fork_end

  neg %eax
  movl %eax, errno
  movl $-1, %eax

fork_end:
  popl %edx
  popl %ecx

  movl %ebp, %esp
  popl %ebp
  ret




ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp

	movl $1, %eax

	pushl %edx
	pushl %ecx

	leal exit_ret, %esi
  pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
exit_ret:

	movl %ebp, %esp
	popl %ebp

  popl %esi

	// error check

  popl %ecx
  popl %edx

	movl %ebp, %esp
	popl %ebp
	ret





ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp

	movl $35, %eax

	pushl %edx
	pushl %ecx

	leal getstats_ret, %esi
  pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
getstats_ret:

	movl %ebp, %esp
	popl %ebp

  popl %esi

	// error check

  popl %ecx
  popl %edx

	movl %ebp, %esp
	popl %ebp
	ret
