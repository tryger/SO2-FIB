#include <sched.h>
#include <sched/rr.h>
#include <mm.h>
#include <io.h>
#include <entry.h>
#include <types.h>

extern quantum;

void init_sched_rr()
{
  update_sched_data = update_sched_data_rr;
  needs_sched = needs_sched_rr;
  update_process_state = update_process_state_rr;
  sched_next = sched_next_rr;
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
      update_stats_run_to_ready(t);
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

  //update_stats_run_to_ready(current());
  //update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
  //update_stats_ready_to_run(tsk);
  //update_stats(&(tsk->p_stats.ready_ticks), &(tsk->p_stats.elapsed_total_ticks));


  task_switch((union task_union *)tsk);
}

