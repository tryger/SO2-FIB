/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <stats.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define QUANTUM_DEFAULT 10

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;

  struct list_head list;
  unsigned long kernel_esp;
  int quantum;
  struct stats p_stats;
  enum state_t process_state;
  int sem_destroyed;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union protected_tasks[NR_TASKS+2];
extern union task_union *task; /* Vector de tasques */
extern struct task_struct *idle_task;
extern struct list_head freequeue;
extern struct list_head readyqueue;
//extern struct semaphore semaphores[NR_SEMAPHORES];


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

int get_new_pid(void);

int get_quantum(struct task_struct *);
void set_quantum(struct task_struct *, int);


void (*sched_next)();
void (*update_process_state)(struct task_struct *t, struct list_head *dest);
int (*needs_sched)();
void (*update_sched_data)();

void update_stats_user_to_system(struct task_struct*);
void update_stats_system_to_user(struct task_struct*);
void update_stats_ready_to_run(struct task_struct*);
void update_stats_run_to_ready(struct task_struct*);

#endif  /* __SCHED_H__ */
