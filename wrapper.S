#include<asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	// Save edx and ecx
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %esi

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
	jns end_write
	neg %eax
	movl %eax, errno
	
	movl $-1, %eax

end_write:
	popl %esi

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
	pushl %esi

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

	popl %esi
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
  pushl %esi

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

  popl %esi
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
  pushl %esi

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
  popl %esi
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
	pushl %esi

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

  popl %esi
  popl %ecx
  popl %edx

	movl %ebp, %esp
	popl %ebp
	ret





ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp

	movl $35, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx

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

  popl %esi
  popl %ecx
  popl %edx
  popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(clone)
	pushl %ebp
	movl %esp, %ebp

	movl $19, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx

	leal clone_ret, %esi
	pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
clone_ret:

	movl %ebp, %esp
	popl %ebp

	test %eax, %eax
	jns end_clone
	neg %eax
	movl %eax, errno
	
	movl $-1, %eax
end_clone:

	popl %esi

	popl %esi
	popl %ecx
	popl %edx
	popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret


ENTRY(sem_init)
	pushl %ebp
	movl %esp, %ebp

	movl $21, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx

	leal sem_init_ret, %esi
	pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
sem_init_ret:

	movl %ebp, %esp
	popl %ebp

	test %eax, %eax
	jns end_sem_init
	neg %eax
	movl %eax, errno

	movl $-1, %eax
end_sem_init:

	popl %esi

	popl %esi
	popl %ecx
	popl %edx
	popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(sem_wait)
	pushl %ebp
	movl %esp, %ebp

	movl $22, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx

	leal sem_wait_ret, %esi
	pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
sem_wait_ret:

	movl %ebp, %esp
	popl %ebp

	test %eax, %eax
	jns end_sem_wait
	neg %eax
	movl %eax, errno

	movl $-1, %eax
end_sem_wait:

	popl %esi

	popl %esi
	popl %ecx
	popl %edx
	popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(sem_signal)
	pushl %ebp
	movl %esp, %ebp

	movl $23, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx

	leal sem_signal_ret, %esi
	pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
sem_signal_ret:

	movl %ebp, %esp
	popl %ebp

	test %eax, %eax
	jns end_sem_signal
	neg %eax
	movl %eax, errno

	movl $-1, %eax
end_sem_signal:

	popl %esi

	popl %esi
	popl %ecx
	popl %edx
	popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret

ENTRY(sem_destroy)
	pushl %ebp
	movl %esp, %ebp

	movl $24, %eax

	pushl %ebx
	pushl %edx
	pushl %ecx
	pushl %esi

	movl 8(%ebp), %ebx

	leal sem_destroy_ret, %esi
	pushl %esi

	pushl %ebp
	movl %esp, %ebp

	sysenter
sem_destroy_ret:

	movl %ebp, %esp
	popl %ebp

	test %eax, %eax
	jns end_sem_destroy
	neg %eax
	movl %eax, errno

	movl $-1, %eax
end_sem_destroy:

	popl %esi

	popl %esi
	popl %ecx
	popl %edx
	popl %ebx

	movl %ebp, %esp
	popl %ebp
	ret
