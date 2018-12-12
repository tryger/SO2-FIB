#include <io.h>
#include <utils.h>
#include <list.h>
#include <devices.h>
#include <sched.h>

// Queue for blocked processes in I/O 
struct list_head blocked;

struct cbuffer keyboard_buffer;

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

int sys_read_keyboard(char *buffer, int size)
{
	int cnt = 0;
	struct task_struct *cur = current();

	if (!list_empty(&blocked)) {
		update_process_state(cur, &blocked);
		sched_next();
	}

	while (1) {
		cnt += cb_read(&keyboard_buffer, buffer+cnt, size);

		if (cnt == size) {
			break;
		} else {
			cur->process_state = ST_BLOCKED;
			list_add(&cur->list, &blocked);
			sched_next();
		}
	}

	return cnt;
}
