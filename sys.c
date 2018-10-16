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
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
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
