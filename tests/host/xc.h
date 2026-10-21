#ifndef TEST_XC_H
#define TEST_XC_H
#include <stdint.h>
#include "platform.h"
/* Registradores simulados. Nao emulam temporizacao nem o pipeline do PIC24. */
typedef union { uint16_t value; struct { unsigned TON:1, TCS:1, TCKPS:2; } bits; } timer_reg_t;
typedef union { uint16_t value; struct { unsigned T1IF:1, INT0IF:1, AD1IF:1; } bits; } flags_reg_t;
typedef union { uint16_t value; struct { unsigned T1IE:1, INT0IE:1, AD1IE:1; } bits; } enable_reg_t;
typedef union { uint16_t value; struct { unsigned T1IP:3, INT0IP:3; } bits; } priority_reg_t;
extern volatile timer_reg_t test_t1, test_t2;
extern volatile flags_reg_t test_ifs;
extern volatile enable_reg_t test_iec;
extern volatile priority_reg_t test_ipc;
extern volatile uint16_t TMR1, PR1, TMR2, PR2, TBLPAG, PSVPAG, CORCON;
#define T1CON test_t1.value
#define T1CONbits test_t1.bits
#define T2CON test_t2.value
#define T2CONbits test_t2.bits
#define IFS0bits test_ifs.bits
#define IEC0bits test_iec.bits
#define IPC0bits test_ipc.bits
#define __builtin_disable_interrupts() ((void)os_irq_save())
void os_host_nop(void);
#define Nop() os_host_nop()
#endif
