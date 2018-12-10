/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <system.h>

#include <errno.h>

#include <entry.h>

#include <semaphore.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
	if (fd != 0 && fd != 1)
		return -EBADF;

	if (fd == 0 && permissions != LECTURA)
		return -EACCES;

	if (fd == 1 && permissions != ESCRIPTURA)
		return -EACCES;

	return 0;
}

int sys_ni_syscall()
{
  return -ENOSYS;
}

int sys_getpid()
{
  return current()->PID;
}

int sys_fork()
{
  int PID, pag;
  struct list_head *lh;
  struct task_struct *tsk, *tsk_cur;
  union task_union *tsku, *tsku_cur;
  page_table_entry *pt, *pt_cur;
  int frames[NUM_PAG_DATA];
  
  /* Obtenim l'estructura del proces actual */
  tsk_cur = current();
  tsku_cur = (union task_union *)tsk_cur;
  pt_cur = get_PT(tsk_cur);

  /* Si no queden PCBs, retornem error */
  if (list_empty(&freequeue))
    return -EAGAIN;

  /* Obtenim un PCB pel nou procés */
  lh = list_first(&freequeue);
  tsk = list_head_to_task_struct(lh);
  tsku = (union task_union *)tsk;
  list_del(lh);

  /* Reservem els frames de data pel nou proces */
  for (pag=0; pag < NUM_PAG_DATA; pag++) {
    frames[pag] = alloc_frame();
    if (frames[pag] == -1) {
      while (pag > 0)
        free_frame(frames[--pag]);

      return -ENOMEM;
    }
  }

  /* Copiem el PCB i l'stack del pare al fill */
  copy_data((void *)tsku_cur, (void *)tsku, sizeof(union task_union));

  allocate_page_dir(tsk);
  pt = get_PT(tsk);

  /* Copiem les entrades de codi a la TP del nou proces, ja que nomes son de lectura */
  for (pag=0; pag < NUM_PAG_CODE; pag++) {
    pt[PAG_LOG_INIT_CODE+pag].entry = pt_cur[PAG_LOG_INIT_CODE+pag].entry;
  }

  /* Ara copiem les pagines de dades */
  for (pag=0; pag < NUM_PAG_DATA; pag++) {
    /* Inicialitzem l'entrada amb el frame reservat anteriorment */
    set_ss_pag(pt, PAG_LOG_INIT_DATA+pag, frames[pag]);

    /* Copiem les dades de la pagina del proces pare al fill 
     * Per fer aixo haurem de reservar una entrada al proces pare
     * que apunti al frame del fill, per a poder-lo indexar
     */
    set_ss_pag(pt_cur, FIRST_FREE_PAGE, frames[pag]);
    copy_data((void *)((PAG_LOG_INIT_DATA+pag)<<12), (void *)(FIRST_FREE_PAGE<<12), PAGE_SIZE);
    del_ss_pag(pt_cur, FIRST_FREE_PAGE);
    /* Fem flush del TLB per a evitar que tradueixi a l'anterior */
    set_cr3(get_DIR(tsk_cur));
  }

  PID = get_new_pid();
  tsk->PID = PID;

  tsku->stack[KERNEL_STACK_SIZE-17] = &ret_from_fork;
  tsku->stack[KERNEL_STACK_SIZE-18] = 0;
  tsk->kernel_esp = &tsku->stack[KERNEL_STACK_SIZE-18];
  
  list_add_tail(&tsk->list, &readyqueue);
  tsk->quantum = QUANTUM_DEFAULT;
  tsk->process_state = ST_READY;

  init_stats(&tsk->p_stats);

  return PID;
}

int sys_clone(void (*function)(void), void *stack)
{
  int PID, pag;
  struct list_head *lh;
  struct task_struct *tsk, *tsk_cur;
  union task_union *tsku, *tsku_cur;
  int frames[NUM_PAG_DATA];

  if (!access_ok(VERIFY_READ, function, 4))
    return -EFAULT;

  if (!access_ok(VERIFY_WRITE, stack, 4))
    return -EFAULT;

  /* Obtenim l'estructura del proces actual */
  tsk_cur = current();
  tsku_cur = (union task_union *)tsk_cur;

  /* Si no queden PCBs, retornem error */
  if (list_empty(&freequeue))
    return -EAGAIN;

  /* Obtenim un PCB pel nou procés */
  lh = list_first(&freequeue);
  tsk = list_head_to_task_struct(lh);
  tsku = (union task_union *)tsk;
  list_del(lh);

  /* Copiem el PCB i l'stack del pare al fill */
  copy_data((void *)tsku_cur, (void *)tsku, sizeof(union task_union));

  PID = get_new_pid();
  tsk->PID = PID;
  ++dir_pages_used[ calculate_dir_index(get_DIR(tsk)) ];

  tsku->stack[KERNEL_STACK_SIZE-17] = &ret_from_fork;
  tsku->stack[KERNEL_STACK_SIZE-18] = 0;
  tsk->kernel_esp = &tsku->stack[KERNEL_STACK_SIZE-18];
  tsku->stack[KERNEL_STACK_SIZE-5] = function;
  tsku->stack[KERNEL_STACK_SIZE-2] = stack;  
  
  list_add_tail(&tsk->list, &readyqueue);
  tsk->quantum = QUANTUM_DEFAULT;
  tsk->process_state = ST_READY;

  init_stats(&tsk->p_stats);

  return PID;
}


void sys_exit()
{
  struct task_struct *cur = current();
  int dir_index = calculate_dir_index(get_DIR(cur));
  int i;
  
  if (--dir_pages_used[dir_index] <= 0) {
  	free_user_pages(cur);
  }

  for (i=0; i < NR_SEMAPHORES; i++) {
  	if (semaphores[i].pid == cur->PID) {
		sys_sem_destroy(i);
	}
  }

  cur->PID = -1;
  list_add_tail(&cur->list, &freequeue);

  sched_next(); 
}


int sys_write(int fd, char* buffer, int size)
{
	int ret;
	char buff[256];
	
	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	
       	if (buffer == NULL)
		return -EFAULT;

	if (size < 0)
		return -EINVAL;

	if (!access_ok(VERIFY_READ, buffer, size))
		return -EACCES;


	ret = 0;
	while (size > 256) {
		copy_from_user(buffer, buff, 256);
		ret += sys_write_console(buffer, 256);
		buffer += 256;
		size -= 256;
	}

	copy_from_user(buffer, buff, size);
	ret += sys_write_console(buffer, size);


	return ret;
}

int sys_read(int fd, char *buffer, int size)
{
	int ret;
	char buff[20];

	if ((ret = check_fd(fd, LECTURA)))
		return ret;

	if (buffer == NULL)
		return -EFAULT;

	if (size < 0)
		return -EINVAL;

	if (!access_ok(VERIFY_READ, buffer, size))
		return -EACCES;


	ret = sys_read_keyboard(buff, 2);

	copy_to_user(buff, buffer, ret);

	return ret;
}

int sys_gettime()
{
	return zeos_ticks;
}

int sys_get_stats(int pid, struct stats *st)
{
	union task_union *tsku;
	
	
	if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats)))
		return -EFAULT;

	if (pid < 0)
		return -EINVAL;

	
	tsku = get_task_union_from_pid(pid);

	if (tsku == NULL)
		return -ESRCH;

	copy_to_user(&tsku->task.p_stats, st, sizeof(struct stats));

	return 0;
}



/* Semaphores */

int sys_sem_init(int n_sem, unsigned int value)
{
	int i;

	if (n_sem < 0 || n_sem >= NR_SEMAPHORES)
		return -EINVAL;

	if (semaphores[n_sem].pid != -1)
		return -EBUSY;

	semaphores[n_sem].pid = sys_getpid();
	semaphores[n_sem].cnt = value;
	INIT_LIST_HEAD(&semaphores[n_sem].blocked);

	return 0;
}

int sys_sem_wait(int n_sem)
{
	struct task_struct *cur = current();

	if (n_sem < 0 || n_sem >= NR_SEMAPHORES)
		return -EINVAL;

	if (semaphores[n_sem].pid == -1)
		return -EINVAL;

	if (--semaphores[n_sem].cnt < 0) {
		cur->process_state = ST_BLOCKED;
		cur->sem_destroyed = 0;

		//list_del(&cur->list);
		list_add_tail(&cur->list, &semaphores[n_sem].blocked);

		sched_next();
	}

	if (cur->sem_destroyed == 1) {
		cur->sem_destroyed = 0;
		return -1;
	} else {
		return 0;
	}
}

int sys_sem_signal(int n_sem)
{
	struct list_head *lh;
	struct task_struct *tsk;
	
	if (n_sem < 0 || n_sem >= NR_SEMAPHORES)
		return -EINVAL;

	if (semaphores[n_sem].pid == -1)
		return -EINVAL;

	++semaphores[n_sem].cnt;

	if (!list_empty(&semaphores[n_sem].blocked)) {
		lh = list_first(&semaphores[n_sem].blocked);
		tsk = list_head_to_task_struct(lh);
		list_del(lh);

		tsk->process_state = ST_READY;

		list_add_tail(lh, &readyqueue);
	}

	return 0;
}

int sys_sem_destroy(int n_sem)
{
	struct list_head *lh;
	struct task_struct *tsk;

	if (n_sem < 0 || n_sem >= NR_SEMAPHORES)
		return -EINVAL;

	if (semaphores[n_sem].pid == -1)
		return -EINVAL;

	if (semaphores[n_sem].pid != sys_getpid())
		return -EPERM;

	if (!list_empty(&semaphores[n_sem].blocked))
		return -EBUSY;

	// Alliberem els processos bloquejats
	while (!list_empty(&semaphores[n_sem].blocked)) {
		lh = list_first(&semaphores[n_sem].blocked);
		tsk = list_head_to_task_struct(lh);
		list_del(lh);

		tsk->process_state = ST_READY;
		tsk->sem_destroyed = 1;


		list_add_tail(lh, &readyqueue);
	}
	
	semaphores[n_sem].pid = -1;

	return 0;
}
