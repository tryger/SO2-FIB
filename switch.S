#include <asm.h>

ENTRY(task_switch)
	pushl %ebp
	movl %esp, %ebp

	pushl %esi
	pushl %edi
	pushl %ebx

	pushl 8(%ebp)
	call inner_task_switch

	popl %ebx

	popl %ebx
	popl %edi
	popl %esi

	movl %ebp, %esp
	popl %ebp
