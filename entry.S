/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

.globl errno


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %ebx; \
      popl %ecx; \
      popl %edx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20


ENTRY(writeMSR)
	pushl %ebp
	movl %esp, %ebp
	
	movl 8(%ebp), %ecx
	movl 12(%ebp), %eax
	xor %edx, %edx
	
	wrmsr

	movl %ebp, %esp
	popl %ebp

	ret



ENTRY(keyboard_handler)
	SAVE_ALL
	EOI
	call keyboard_routine
	RESTORE_ALL
	iret

// sysenter
ENTRY(syscall_handler_sysenter)
	push $__USER_DS
	push %ebp // user stack
	pushfl
	push $__USER_CS
	push 4(%ebp) // user return address

	SAVE_ALL

	// update ticks
	call current
	pushl %eax
	call update_stats_user_to_system
	popl %eax
	movl 0x18(%esp), %eax
	
	
	cmpl $0, %eax
	jl sysenter_err
	cmpl $MAX_SYSCALL, %eax
	jg sysenter_err

	call *sys_call_table(, %eax, 4)
	jmp sysenter_fin

sysenter_err:
	movl $-ENOSYS, %eax

sysenter_fin:
	movl %eax, 0x18(%esp)

	call current
	pushl %eax
	call update_stats_system_to_user
	popl %eax

	RESTORE_ALL

	movl (%esp), %edx // user return address
	movl 12(%esp), %ecx // user stack

  sti
	sysexit


ENTRY(clock_handler)
	SAVE_ALL
	EOI
	call clock_routine
	RESTORE_ALL
	iret


ENTRY(ret_from_fork)
  RESTORE_ALL
  xor %eax, %eax
  iret
