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

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF;
  if (permissions!=ESCRIPTURA) return -EACCES;
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
  copy_data((void *)tsku_cur, (void *)tsku, KERNEL_STACK_SIZE*sizeof(DWord));

  allocate_DIR(tsk);
  

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
    /* Fem flush del TLB per a que tradueixi a l'anterior */
    set_cr3(get_DIR(tsk_cur));
  }

  /*__asm__ __volatile__ (
    "movl %%ebp, %0"
  );*/

  tsku->stack[KERNEL_STACK_SIZE-18] = &ret_from_fork;
  tsku->stack[KERNEL_STACK_SIZE-19] = 0;
  tsk->kernel_esp = &tsku->stack[KERNEL_STACK_SIZE-19];

  PID = getNewPID();
  tsk->PID = PID;

  return PID;
}

void sys_exit()
{  
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

	if (access_ok(VERIFY_READ, buffer, size) == 0)
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

int sys_gettime()
{
	return zeos_ticks;
}
