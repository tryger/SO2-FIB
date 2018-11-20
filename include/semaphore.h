#ifndef __H_SEMAPHORE__
#define __H_SEMAPHORE__

#include <list.h>

#define NR_SEMAPHORES 20

struct semaphore {
	int pid;
	int cnt;
	struct list_head blocked;
};

struct semaphore semaphores[NR_SEMAPHORES];

#endif //__H_SEMAPHORE__
