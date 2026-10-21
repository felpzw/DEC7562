#include "timer.h"
#include "kernel.h"
#include "scheduler.h"
#include <xc.h>

extern queue_t ReadyQueue;
extern uint8_t task_running;
extern uint16_t *task_running_tos;
extern volatile uint32_t os_ticks;


void config_timer_0(void)
{
    T1CON = 0;
    T1CONbits.TCS = 0;          // Clock interno 
    T1CONbits.TCKPS = 1;        // Preescaler 1:8
    IFS0bits.T1IF = 0;          // Flag
    IEC0bits.T1IE = 1;          // Atividação da interrupção por timer
    IPC0bits.T1IP = 4;          // Prioridade
    TMR1 = 0;
    PR1 = (FCY / 8UL / 1000UL) * OS_TICK_MS - 1;
}

void OS_INTERRUPT _T1Interrupt(void)
{
    SAVE_CONTEXT();
    
    IFS0bits.T1IF = 0;
    
    // Tarefa que está deixando a CPU
    ReadyQueue.tasks[task_running].sp = task_running_tos;
    if (ReadyQueue.tasks[task_running].task_state == RUNNING)
        ReadyQueue.tasks[task_running].task_state = READY;
    
    ++os_ticks;
    delay_release();
    
    task_running = scheduler();
    
    // Tarefa que irá assumir a CPU
    task_running_tos = ReadyQueue.tasks[task_running].sp;
    ReadyQueue.tasks[task_running].task_state = RUNNING;
    
    RESTORE_CONTEXT();    
}

