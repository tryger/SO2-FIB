#ifndef __SCHED_RR_H__
#define __SCHED_RR_H__

void init_sched_rr();

void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

#endif //__SCHED_RR_H__
