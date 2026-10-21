#include "sync.h"
#include "int.h"
#include <stddef.h>

extern queue_t ReadyQueue;
extern uint8_t task_running;

void sem_init(sem_t *s, uint8_t value)
{
    s->sem_count = value;
    s->s_queue_input = 0;
    s->s_queue_output = 0;
}

void sem_wait(sem_t *s)
{
    irq_state_t state = os_irq_save();
    if (--s->sem_count < 0) {
        s->sem_queue[s->s_queue_input] = task_running;
        s->s_queue_input = (s->s_queue_input + 1) % MAX_TASKS;
        ReadyQueue.tasks[task_running].task_state = BLOCKED;
        CONTEXT_SWITCH();
    }
    os_irq_restore(state);
}

void sem_post(sem_t *s)
{
    irq_state_t state = os_irq_save();
    if (++s->sem_count <= 0) {
        ReadyQueue.tasks[s->sem_queue[s->s_queue_output]].task_state = READY;
        s->s_queue_output = (s->s_queue_output + 1) % MAX_TASKS;
        CONTEXT_SWITCH();
    }
    os_irq_restore(state);
}

/* Cada aresta tarefa -> mutex -> proprietario representa uma doacao.
 * Reiniciar pelas prioridades base remove doacoes antigas; ate N passagens
 * propagam a maior prioridade por uma cadeia de N tarefas. */
static void update_priorities(void)
{
    for (uint8_t i = 0; i < ReadyQueue.queue_size; ++i)
        ReadyQueue.tasks[i].task_prior = ReadyQueue.tasks[i].base_prior;
    for (uint8_t pass = 0; pass < ReadyQueue.queue_size; ++pass) {
        bool changed = false;
        for (uint8_t i = 0; i < ReadyQueue.queue_size; ++i) {
            tcb_t *waiter = &ReadyQueue.tasks[i];
            mutex_t *m = waiter->waiting_mutex;
            if (m != NULL && m->owner < ReadyQueue.queue_size &&
                ReadyQueue.tasks[m->owner].task_prior < waiter->task_prior) {
                ReadyQueue.tasks[m->owner].task_prior = waiter->task_prior;
                changed = true;
            }
        }
        if (!changed) break;
    }
}

void mutex_init(mutex_t *m)
{
    m->owner = OS_NO_TASK;
    m->waiter_count = 0;
}

bool mutex_lock(mutex_t *m)
{
    uint8_t self = task_running;
    irq_state_t state = os_irq_save();
    if (m == NULL || self == 0 || self >= ReadyQueue.queue_size) {
        os_irq_restore(state);
        return false;
    }
    if (m->owner == OS_NO_TASK) {
        m->owner = self;
        os_irq_restore(state);
        return true;
    }
    /* Mutex nao recursivo: rejeita tambem ciclos de espera. */
    uint8_t owner = m->owner;
    for (uint8_t n = 0; n < ReadyQueue.queue_size; ++n) {
        if (owner == self || owner >= ReadyQueue.queue_size) {
            os_irq_restore(state);
            return false;
        }
        mutex_t *waiting = ReadyQueue.tasks[owner].waiting_mutex;
        if (waiting == NULL) break;
        owner = waiting->owner;
    }
    if (m->waiter_count == MAX_TASKS) {
        os_irq_restore(state);
        return false;
    }
    m->waiters[m->waiter_count++] = self;
    ReadyQueue.tasks[self].waiting_mutex = m;
    ReadyQueue.tasks[self].task_state = BLOCKED;
    update_priorities();
    CONTEXT_SWITCH();
    os_irq_restore(state);
    /* A interrupcao suspende esta chamada; unlock transfere a posse antes
     * de tornar a tarefa READY. Ao retomar, ela ja possui o mutex. */
    return m->owner == self;
}

bool mutex_unlock(mutex_t *m)
{
    irq_state_t state = os_irq_save();
    if (m == NULL || m->owner == OS_NO_TASK || m->owner != task_running) {
        os_irq_restore(state);
        return false;
    }
    if (m->waiter_count != 0) {
        uint8_t best = 0;
        for (uint8_t i = 1; i < m->waiter_count; ++i) {
            if (ReadyQueue.tasks[m->waiters[i]].task_prior >
                ReadyQueue.tasks[m->waiters[best]].task_prior) best = i;
        }
        m->owner = m->waiters[best];
        for (uint8_t i = best + 1; i < m->waiter_count; ++i)
            m->waiters[i - 1] = m->waiters[i];
        --m->waiter_count;
        ReadyQueue.tasks[m->owner].waiting_mutex = NULL;
        ReadyQueue.tasks[m->owner].task_state = READY;
    } else {
        m->owner = OS_NO_TASK;
    }
    update_priorities();
    CONTEXT_SWITCH();
    os_irq_restore(state);
    return true;
}
