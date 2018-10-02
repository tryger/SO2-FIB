/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void keyboard_handler();

void system_call_handler(); //UNUSED

void syscall_handler_sysenter();

void clock_handler();

void writeMSR(int reg, int val);

#endif  /* __ENTRY_H__ */
