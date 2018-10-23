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


void init_stats(struct task_struct *t)
{
  t->p_stats.user_ticks = 0;
  t->p_stats.system_ticks = 0;
  t->p_stats.blocked_ticks = 0;
  t->p_stats.ready_ticks = 0;
  t->p_stats.elapsed_total_ticks = get_ticks();
  t->p_stats.total_trans = 0;
  t->p_stats.remaining_ticks = get_ticks();
}


void init_idle (void)
{
	struct list_head *idle_list_pointer = list_first(&freequeue);
	list_del(idle_list_pointer);
	idle_task = list_head_to_task_struct(idle_list_pointer);

	union task_union *idle_union_stack = (union task_union *)idle_task;

	idle_task->PID = getNewPID();
  idle_task->quantum = QUANTUM_DEFAULT;

  init_stats(idle_task);

	idle_union_stack->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	idle_union_stack->stack[KERNEL_STACK_SIZE-2] = 0;
	idle_union_stack->task.kernel_esp = (unsigned long)&idle_union_stack->stack[KERNEL_STACK_SIZE-2];
}
void init_task1(void)
{
	struct list_head *task1_list_pointer = list_first(&freequeue);
	list_del(task1_list_pointer);
	struct task_struct *task1_task_struct = list_head_to_task_struct(task1_list_pointer);
	union task_union *task1_union_stack = (union task_union *)task1_task_struct;

	task1_task_struct->PID = getNewPID();
	task1_task_struct->quantum = QUANTUM_DEFAULT;
	task1_task_struct->process_state = ST_RUN;
  quantum = task1_task_struct->quantum;

  init_stats(task1_task_struct);

  allocate_DIR(task1_task_struct);
	set_user_pages(task1_task_struct);

	tss.esp0 = &task1_union_stack->stack[KERNEL_STACK_SIZE-1];
	writeMSR(0x175, tss.esp0);

	set_cr3(task1_task_struct->dir_pages_baseAddr);
}

int getNewPID(void) {
  return nextPID++;
}


void init_sched_rr()
{
  update_sched_data = update_sched_data_rr;
  needs_sched = needs_sched_rr;
  update_process_state = update_process_state_rr;
  sched_next = sched_next_rr;
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

int get_quantum(struct task_struct *t)
{
  return t->quantum;
}

void set_quantum(struct task_struct *t, int q)
{
  t->quantum = q;
}


void update_stats(unsigned long *v, unsigned long *elapsed)
{
  unsigned long ct = get_ticks();
  *v += ct - *elapsed;
  *elapsed = get_ticks();
}


int needs_sched_rr()
{
  if ((quantum == 0) && (!list_empty(&readyqueue)))
    return 1;

  if (quantum == 0)
    quantum = get_quantum(current());

  return 0;
}

void update_sched_data_rr()
{
  --quantum;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{
  
  list_add_tail(&t->list, dest);

  if (dest != NULL) {
    if (dest != &readyqueue) {
      t->process_state = ST_BLOCKED;
    } else {
      update_stats(&(t->p_stats.system_ticks), &(t->p_stats.elapsed_total_ticks));
      t->process_state = ST_READY;
    }
  } else {
    t->process_state = ST_RUN;
  }

}

void sched_next_rr()
{
  struct list_head *lh;
  struct task_struct *tsk;


  if (!list_empty(&readyqueue)) {
    lh = list_first(&readyqueue);
    tsk = list_head_to_task_struct(lh);
    list_del(lh);
  } else {
    tsk = idle_task;
  }

  tsk->process_state = ST_RUN;
  quantum = get_quantum(tsk);

  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
  update_stats(&(tsk->p_stats.ready_ticks), &(tsk->p_stats.elapsed_total_ticks));
  tsk->p_stats.total_trans;

  task_switch((union task_union *)tsk);
}


void schedule()
{
  update_sched_data();
  
  if (needs_sched()) {
    update_process_state(current(), &readyqueue);
    sched_next();
  }
}
