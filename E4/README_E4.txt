READ

Test[0]: [testread1] READ(fd is negative)
Test[1]: [testread3] READ(fd is closed)
Test[2]: [testread4] READ(fd is WRONLY)
Test[3]: [testread5] READ(read from fd==0  on a null pointer buffer)
Test[4]: [testread8] READ(read 1 char from keyboard (fd == 0))
Test[5]: [testread9] READ(read 4 char from keyboard (fd == 0))
Test[6]: [testread19] READ(read from keyboard (fd == 0) more bytes than keyboard buffer capacity)
Test[7]: [testStatsBlocked] READ(get stats of a process blocked by reading keyboard ): sem_init, sem_destroy, sem_signal, sem_wait, fork, exit, get_stats
Test[8]: [testread6] READ(read from fd==0 on buffer in kernel address space)
Test[9]: [testread7] READ(read from fd==0 on an buffer outside user address space )
Test[10]: [testread2] READ(fd greater than MAX FDs)

SBRK

Test[0]: [testsbrk1] SBRK(0): write
Test[1]: [testsbrk2] SBRK(10): write
Test[2]: [testsbrk3] SBRK(10)+use: write
Test[3]: [testsbrk4] SBRK(10)+fork: write, sem_init, sem_destroy, sem_signal, sem_wait, fork, exit
Test[4]: [testsbrk5] SBRK(10)+thread: write, sem_init, sem_destroy, sem_signal, sem_wait, clone, exit
Test[5]: [testsbrk7] SBRK(max): write
Test[6]: [testsbrk8] SBRK(-max): write
Test[7]: [testsbrk9] SBRK(1)+SBRK(4096): write

