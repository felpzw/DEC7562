#include "int.h"
#include "kernel.h"
#include "scheduler.h"
#include <xc.h>


extern queue_t ReadyQueue;
extern uint8_t task_running;
extern uint16_t *task_running_tos;


void config_int0(void)
{
    IEC0bits.INT0IE = 1;
    IFS0bits.INT0IF = 0;
    IPC0bits.INT0IP = 4;
}

void OS_INTERRUPT _INT0Interrupt(void)
{
    SAVE_CONTEXT();
    
    IFS0bits.INT0IF = 0;
    
    // Tarefa que está deixando a CPU
    ReadyQueue.tasks[task_running].sp = task_running_tos;
    if (ReadyQueue.tasks[task_running].task_state == RUNNING)
        ReadyQueue.tasks[task_running].task_state = READY;
    
    task_running = scheduler();
    
    // Tarefa que irá assumir a CPU
    task_running_tos = ReadyQueue.tasks[task_running].sp;
    ReadyQueue.tasks[task_running].task_state = RUNNING;
    
    RESTORE_CONTEXT();    
}
