#include <libc.h>

char buff[24];

int pid;


int add2(int, int);

long inner( long n)
{
	int  i;
	long suma;
	
	suma = 0;
	for (i=0; i<n; i++) suma = suma + i;

	return suma;
}

long outer(long n)
{
	int i;
	long acum;

	acum = 0;
	for (i=0; i<n; i++) acum = acum + inner(i);

	return acum;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  runjp(8, 8);

  while(1);

  return 0;
}


int add(int par1, int par2)
{
	return par1 + par2;
}
