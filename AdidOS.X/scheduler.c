#include "scheduler.h"
#include "kernel.h"

extern queue_t ReadyQueue;
extern uint8_t task_running;

uint8_t scheduler(void)
{
#if DEFAULT_SCHEDULER == RR_SCHEDULER
    return rr_scheduler();
#elif DEFAULT_SCHEDULER == PRIOR_SCHEDULER
    return prior_scheduler();
#else
#error Escalonador desconhecido
#endif
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
