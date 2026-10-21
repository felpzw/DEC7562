#include "kernel.h"
#include <stdio.h>
#include "user_app.h"
#include "timer.h"
#include "int.h"
#include <xc.h>
#include "mem.h"
#include "scheduler.h"

// Variáveis globais
queue_t ReadyQueue;
uint8_t task_running = 0;
uint16_t *task_running_tos = NULL;
volatile uint32_t os_ticks = 0;

void os_config(void)
{
    /* Nenhuma interrupcao pode observar a fila parcialmente inicializada. */
    __builtin_disable_interrupts();
    ReadyQueue.queue_size   = 0;
    task_running = 0;
    os_ticks = 0;
    config_timer_0();
    config_int0();
    os_heap_init();
    config_user_app();
    
    create_task(idle, 0);
}

void os_start_scheduler(void)
{
    __builtin_disable_interrupts();
    task_running = scheduler();
    ReadyQueue.tasks[task_running].task_state = RUNNING;
    task_running_tos = ReadyQueue.tasks[task_running].sp;
    T1CONbits.TON = 1;
    RESTORE_CONTEXT();
}

bool create_task(callback task, uint8_t prior)
{
    if (task == NULL || ReadyQueue.queue_size >= MAX_TASKS + 1) return false;
    tcb_t *new_task = &ReadyQueue.tasks[ReadyQueue.queue_size];
    
    // Preencher a tcb
    new_task->task_id       = ReadyQueue.queue_size+1;
    new_task->task_func     = task;
    new_task->task_prior    = prior;
    new_task->base_prior    = prior;
    new_task->waiting_mutex = NULL;
    new_task->task_state    = READY;
    new_task->task_delay    = 0;
    
    // Ajustar o SP
    new_task->sp            = &new_task->task_stack[0];
    
    // Inicializar a pilha
    init_stack(new_task);
    
    // Insere na fila de aptos    
    ReadyQueue.queue_size++;
    return true;
}

void init_stack(tcb_t *task)
{
    uint16_t pc_low, pc_high, *sp;
    
    sp = task->sp;
    
    pc_low  = (uint16_t)(uintptr_t)task->task_func;
    pc_high = 0x0000;
    
    *sp++ = pc_low;
    *sp++ = pc_high;
    
    // Inicializar os Ws (W0 até W14)
    for (int i = 0; i < 15; i++) {
        *sp++ = 0x0000;
    }
  
    *sp++ = 0x0000; // RCOUNT
    *sp++ = TBLPAG; // TBLPAG
    *sp++ = PSVPAG; // PSVPAG
    *sp++ = CORCON; // CORCON
    
    task->sp = sp;
}

void task_yield(void)
{   
    irq_state_t state = os_irq_save();

    ReadyQueue.tasks[task_running].task_state = READY;
    CONTEXT_SWITCH();

    os_irq_restore(state);
}

void task_delay(uint16_t time)
{
    if (time == 0) { task_yield(); return; }
    irq_state_t state = os_irq_save();
    
    ReadyQueue.tasks[task_running].task_delay = time;
    ReadyQueue.tasks[task_running].task_state = WAITING;
    CONTEXT_SWITCH();
    
    os_irq_restore(state);
}

void delay_release(void)
{
    for (int i = 0; i < ReadyQueue.queue_size; i++) {
        if (ReadyQueue.tasks[i].task_state == WAITING) {
            if (ReadyQueue.tasks[i].task_delay > 0) {
                ReadyQueue.tasks[i].task_delay--;
            }
            if (ReadyQueue.tasks[i].task_delay == 0) {
                ReadyQueue.tasks[i].task_state = READY;
            }
        }
    }
}

uint32_t os_get_ticks(void)
{
    irq_state_t state = os_irq_save();
    uint32_t ticks = os_ticks;
    os_irq_restore(state);
    return ticks;
}

uint8_t task_get_priority(void)
{
    return ReadyQueue.tasks[task_running].task_prior;
}

TASK idle(void)
{
    while (1) {
        Nop();
    }
}
