/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <entry.h>
#include <types.h>

#include <sched/rr.h>

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

int nextPID;

int quantum;


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


void init_stats(struct stats *st)
{
	st->user_ticks = 0;
	st->system_ticks = 0;
	st->blocked_ticks = 0;
	st->ready_ticks = 0;
	st->elapsed_total_ticks = get_ticks();
	st->total_trans = 0;
	st->remaining_ticks = 0;
}


void init_idle (void)
{
	struct list_head *lh = list_first(&freequeue);
	struct task_struct *tsk;
	union task_union *tsku;

	list_del(lh);
       	tsk = list_head_to_task_struct(lh);
       	tsku = (union task_union *)tsk;

	tsk->PID = get_new_pid();

	/* Init stats */
	tsk->quantum = QUANTUM_DEFAULT;
	tsk->process_state = ST_READY;
	init_stats(&tsk->p_stats);

	tsku->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	tsku->stack[KERNEL_STACK_SIZE-2] = 0;
	tsku->task.kernel_esp = (unsigned long)&tsku->stack[KERNEL_STACK_SIZE-2];
}
void init_task1(void)
{
	struct list_head *lh = list_first(&freequeue);
	list_del(lh);
	struct task_struct *tsk = list_head_to_task_struct(lh);
	union task_union *tsku = (union task_union *)tsk;

	tsk->PID = get_new_pid();
	
	/* Init stats*/
	tsk->quantum = QUANTUM_DEFAULT;
	tsk->process_state = ST_RUN;
	init_stats(&tsk->p_stats);
	tsk->p_stats.remaining_ticks = tsk->quantum;

	quantum = tsk->quantum;

	allocate_DIR(tsk);
	set_user_pages(tsk);

	tss.esp0 = &tsku->stack[KERNEL_STACK_SIZE-1];
	writeMSR(0x175, tss.esp0);

	set_cr3(tsk->dir_pages_baseAddr);
}

int get_new_pid(void) {
  return nextPID++;
}


void init_sched(){
  nextPID = 0;

	init_freequeue();
	init_readyqueue();

  init_sched_rr();
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
    task[i].task.PID = -1;
		list_add_tail(&task[i].task.list,&freequeue);
	}
}


void init_readyqueue (void) {
	INIT_LIST_HEAD(&readyqueue);
}

/*void task_switch(union task_union *new)
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
}*/

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

int get_quantum(struct task_struct *t)
{
  return t->quantum;
}

void set_quantum(struct task_struct *t, int q)
{
  t->quantum = q;
}


void update_stats_user_to_system(struct task_struct *tsk)
{
	update_stats(&tsk->p_stats.user_ticks, &tsk->p_stats.elapsed_total_ticks);
}

void update_stats_system_to_user(struct task_struct *tsk)
{
	update_stats(&tsk->p_stats.system_ticks, &tsk->p_stats.elapsed_total_ticks);
}

void update_stats_run_to_ready(struct task_struct *tsk)
{
	update_stats(&tsk->p_stats.system_ticks, &tsk->p_stats.elapsed_total_ticks);
}

void update_stats_ready_to_run(struct task_struct *tsk)
{
	update_stats(&tsk->p_stats.ready_ticks, &tsk->p_stats.elapsed_total_ticks);
}

void update_stats(unsigned long *v, unsigned long *elapsed)
{
  *v += get_ticks() - *elapsed;
  *elapsed = get_ticks();
}

void schedule()
{
  update_sched_data();
  
  if (needs_sched()) {
    update_process_state(current(), &readyqueue);
    sched_next();
  }
}
