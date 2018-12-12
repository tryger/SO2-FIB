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

	ret

ENTRY(inner_task_switch)
	pushl %ebp
	movl %esp, %ebp

	call current
	movl %eax, %ebx // cur
	movl 8(%ebp), %esi //new

	pushl %esi
	call kernel_esp

	// set the kernel esp for fast syscalls
	pushl %eax
	pushl $0x175
	call writeMSR

	// set cr3 for using the page directory of the process

	// If the process is a thread skip change cr3
	movl 4(%esi), %eax
	cmp 4(%ebx), %eax
	je skip_change_baseAddr

	// if process is idle_task, skip change cr3
	leal idle_task, %eax
	cmp %esi, (%eax)
	je skip_change_baseAddr

	pushl 4(%esi) // new->dir_page_baseAddr
	call set_cr3

skip_change_baseAddr:

	// context switch
	movl %ebp, 16(%ebx)
	movl 16(%esi), %esp
	popl %ebp
	ret
