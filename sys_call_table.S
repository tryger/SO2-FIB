#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall //0
	.long sys_exit //1
	.long sys_fork //2
	.long sys_read //3
	.long sys_write //4
	.long sys_ni_syscall //5
	.long sys_ni_syscall //6
	.long sys_ni_syscall //7
	.long sys_ni_syscall //8
	.long sys_ni_syscall //9
	.long sys_gettime //10
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall //15
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_clone
	.long sys_getpid //20
	.long sys_sem_init
	.long sys_sem_wait
	.long sys_sem_signal
	.long sys_sem_destroy
	.long sys_ni_syscall //25
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall //30
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_get_stats //35

.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4
