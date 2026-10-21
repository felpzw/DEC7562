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
typedef union { uint16_t value; struct { unsigned OCM:3, OCTSEL:1; } bits; } oc_reg_t;
typedef union { uint16_t value; struct { unsigned DONE:1, SAMP:1, ASAM:1, reserved:2, SSRC:3, FORM:2, spare:5, ADON:1; } bits; } adc1_reg_t;
typedef union { uint16_t value; struct { unsigned ADCS:8, SAMC:5; } bits; } adc3_reg_t;
typedef union { uint16_t value; struct { unsigned CH0SA:4; } bits; } adchs_reg_t;
typedef union { uint16_t value; struct { unsigned TRISD0:1; } bits; } trisd_reg_t;
typedef union { uint16_t value; struct { unsigned LATD0:1; } bits; } latd_reg_t;
typedef union { uint16_t value; struct { unsigned TRISE0:1, TRISE1:1, TRISE2:1; } bits; } trise_reg_t;
typedef union { uint16_t value; struct { unsigned LATE0:1, LATE1:1, LATE2:1; } bits; } late_reg_t;
typedef union { uint16_t value; struct { unsigned unused:3, TRISF3:1; } bits; } trisf_reg_t;
typedef union { uint16_t value; struct { unsigned PDSEL:2, STSEL:1, BRGH:1, UARTEN:1; } bits; } uartmode_reg_t;
typedef union { uint16_t value; struct { unsigned UTXEN:1, UTXBF:1; } bits; } uartsta_reg_t;
extern volatile oc_reg_t test_oc;
extern volatile adc1_reg_t test_adc1;
extern volatile adc3_reg_t test_adc3;
extern volatile adchs_reg_t test_adchs;
extern volatile trisd_reg_t test_trisd;
extern volatile latd_reg_t test_latd;
extern volatile trise_reg_t test_trise;
extern volatile late_reg_t test_late;
extern volatile trisf_reg_t test_trisf;
extern volatile uartmode_reg_t test_uartmode;
extern volatile uartsta_reg_t test_uartsta;
extern volatile uint16_t OC1R, OC1RS, AD1CON2, AD1CSSL, AD1PCFG, TRISB, ADC1BUF0;
extern volatile uint16_t U1BRG, U1TXREG;
extern struct { unsigned RCDIV:3; } CLKDIVbits;
#define OC1CON test_oc.value
#define OC1CONbits test_oc.bits
#define AD1CON1 test_adc1.value
#define AD1CON1bits test_adc1.bits
#define AD1CON3 test_adc3.value
#define AD1CON3bits test_adc3.bits
#define AD1CHS test_adchs.value
#define AD1CHSbits test_adchs.bits
#define TRISDbits test_trisd.bits
#define LATDbits test_latd.bits
#define TRISEbits test_trise.bits
#define LATEbits test_late.bits
#define TRISFbits test_trisf.bits
#define U1MODE test_uartmode.value
#define U1MODEbits test_uartmode.bits
#define U1STA test_uartsta.value
#define U1STAbits test_uartsta.bits
#endif
