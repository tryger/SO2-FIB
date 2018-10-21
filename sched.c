/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <entry.h>
#include <types.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}

extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;

struct task_struct * idle_task;



/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	printk("IDLE!\n");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	struct list_head *idle_list_pointer = list_first(&freequeue);
	list_del(idle_list_pointer);
	idle_task = list_head_to_task_struct(idle_list_pointer);

	union task_union *idle_union_stack = (union task_union *)idle_task;

	idle_task->PID = 0;
	idle_union_stack->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	idle_union_stack->stack[KERNEL_STACK_SIZE-2] = 0;
	idle_union_stack->task.kernel_esp = (unsigned long)&idle_union_stack->stack[1022];		
}

void init_task1(void)
{
	struct list_head *task1_list_pointer = list_first(&freequeue);
	list_del(task1_list_pointer);
	struct task_struct *task1_task_struct = list_head_to_task_struct(task1_list_pointer);
	union task_union *task1_union_stack = (union task_union *)task1_task_struct;

	task1_task_struct->PID = 1;
	allocate_DIR(task1_task_struct);
	set_user_pages(task1_task_struct);

	//tss.esp0 = (Word)&task1_union_stack->stack[KERNEL_STACK_SIZE-1];
	writeMSR(0x175, tss.esp0);

	set_cr3(task1_task_struct->dir_pages_baseAddr);
}


void init_sched(){
	init_freequeue();
	init_readyqueue();
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


void init_freequeue(void)
{
	INIT_LIST_HEAD(&freequeue);

	int i;
	for (i = 0; i < NR_TASKS; ++i) {
		list_add_tail(&task[i].task.list,&freequeue);
	}
}


void init_readyqueue (void) {
	INIT_LIST_HEAD(&readyqueue);
}

void task_switch(union task_union *new)
{
	__asm__ __volatile__ (
		"pushl %esi;"
		"pushl %edi;"
		"pushl %ebx;"
	);

	inner_task_switch(new);

	__asm__ __volatile__ (
		"popl %ebx;"
		"popl %edi;"
		"popl %esi;"		
	);
}

void inner_task_switch(union task_union *new)
{
	struct task_struct *cur = current();
	
	tss.esp0 = new->task.kernel_esp;
	writeMSR(0x175, tss.esp0);

	set_cr3(new->task.dir_pages_baseAddr);

	void *cur_esp = cur->kernel_esp;
	void *new_esp = new->task.kernel_esp;
	__asm__ __volatile__ (
		"movl %%ebp,%0;"
		"movl %1,%%esp;"
		"popl %%ebp;"
		"ret;"
		:
		: "g" (cur_esp), "g" (new_esp)
	);
}
