#include "scheduler.h"
#include "kernel.h"

extern queue_t ReadyQueue;
extern uint8_t task_running;
extern volatile uint32_t os_ticks;
volatile scheduler_trace_entry_t scheduler_trace[SCHEDULER_TRACE_SIZE];
volatile uint8_t scheduler_trace_position;

uint8_t scheduler(void)
{
    uint8_t selected;
#if DEFAULT_SCHEDULER == RR_SCHEDULER
    selected = rr_scheduler();
#elif DEFAULT_SCHEDULER == PRIOR_SCHEDULER
    selected = prior_scheduler();
#else
#error Escalonador desconhecido
#endif
    if (selected != OS_NO_TASK) {
        scheduler_trace[scheduler_trace_position].tick = (uint16_t)os_ticks;
        scheduler_trace[scheduler_trace_position].task_id = ReadyQueue.tasks[selected].task_id;
        scheduler_trace[scheduler_trace_position].priority = ReadyQueue.tasks[selected].task_prior;
        scheduler_trace_position = (scheduler_trace_position + 1) % SCHEDULER_TRACE_SIZE;
    }
    return selected;
}

uint8_t rr_scheduler(void)
{
    uint8_t next = task_running;
    if (ReadyQueue.queue_size == 0) return OS_NO_TASK;
    for (uint8_t count = 0; count < ReadyQueue.queue_size; ++count) {
        next = (next + 1) % ReadyQueue.queue_size;
        if (ReadyQueue.tasks[next].task_state == READY &&
            ReadyQueue.tasks[next].task_func != idle) return next;
    }
    return 0; /* idle ocupa sempre a posicao zero. */
}

uint8_t prior_scheduler(void)
{
    uint8_t next = task_running;
    uint8_t selected = 0;
    bool found = false;
    if (ReadyQueue.queue_size == 0) return OS_NO_TASK;
    /* A primeira candidata vem apos a atual: RR nos empates. */
    for (uint8_t count = 0; count < ReadyQueue.queue_size; ++count) {
        next = (next + 1) % ReadyQueue.queue_size;
        tcb_t *task = &ReadyQueue.tasks[next];
        if (task->task_state != READY || task->task_func == idle) continue;
        if (!found || task->task_prior > ReadyQueue.tasks[selected].task_prior) {
            selected = next;
            found = true;
        }
    }
    return selected;
}
