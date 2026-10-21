#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "scheduler.h"
#include "sync.h"
#include "mem.h"
#include "message.h"
#include "timer.h"
#include "int.h"
#include <xc.h>

extern queue_t ReadyQueue;
extern uint8_t task_running;
extern volatile uint32_t os_ticks;
volatile timer_reg_t test_t1, test_t2;
volatile flags_reg_t test_ifs;
volatile enable_reg_t test_iec;
volatile priority_reg_t test_ipc;
volatile uint16_t TMR1, PR1, TMR2, PR2, TBLPAG, PSVPAG, CORCON;
static irq_state_t masked;
static bool pending, expect_block, delayed;
static unsigned switches;
static jmp_buf blocked;
static void (*resume_hook)(void);

irq_state_t os_irq_save(void) { irq_state_t old = masked; masked = 1; return old; }
void os_host_context_switch(void) { assert(masked); pending = true; }
void os_irq_restore(irq_state_t state)
{
    masked = state;
    if (!masked && pending && !delayed) {
        pending = false;
        ++switches;
        if (resume_hook != NULL) { void (*hook)(void) = resume_hook; resume_hook = NULL; hook(); }
        else if (expect_block && ReadyQueue.tasks[task_running].task_state == BLOCKED)
            longjmp(blocked, 1);
    }
}
void os_host_nop(void)
{
    if (pending && delayed && !masked) {
        pending = false;
        delayed = false;
        _INT0Interrupt();
    }
}
bool config_user_app(void) { return true; }
static void task(void) {}

static void setup(void)
{
    memset(&ReadyQueue, 0, sizeof ReadyQueue);
    masked = pending = expect_block = false;
    delayed = false;
    resume_hook = NULL;
    switches = 0;
    task_running = 0;
    assert(create_task(idle, 0));
    for (uint8_t i = 1; i <= MAX_TASKS; ++i) assert(create_task(task, i));
    os_heap_init();
}
static void running(uint8_t index)
{
    for (uint8_t i = 0; i < ReadyQueue.queue_size; ++i)
        if (ReadyQueue.tasks[i].task_state == RUNNING) ReadyQueue.tasks[i].task_state = READY;
    task_running = index;
    ReadyQueue.tasks[index].task_state = RUNNING;
}
static void block_on(mutex_t *mutex, uint8_t index)
{
    running(index);
    expect_block = true;
    if (setjmp(blocked) == 0) { (void)mutex_lock(mutex); assert(!"lock retornou antes da liberacao"); }
    expect_block = false;
    assert(ReadyQueue.tasks[index].task_state == BLOCKED);
    assert(!masked);
}

static void test_scheduler(void)
{
    setup();
    assert(!create_task(task, 1));
    assert(!create_task(NULL, 1));
    assert(prior_scheduler() == 5);
    ReadyQueue.tasks[3].task_prior = 5;
    task_running = 3;
    assert(prior_scheduler() == 5);
    task_running = 5;
    assert(prior_scheduler() == 3);
    ReadyQueue.tasks[3].task_state = BLOCKED;
    ReadyQueue.tasks[5].task_state = WAITING;
    assert(prior_scheduler() == 4);
    for (uint8_t i = 1; i <= MAX_TASKS; ++i) ReadyQueue.tasks[i].task_state = BLOCKED;
    assert(prior_scheduler() == 0);
    assert(rr_scheduler() == 0);
    ReadyQueue.tasks[2].task_state = READY;
    ReadyQueue.tasks[2].task_prior = 0;
    assert(prior_scheduler() == 2);
    assert(rr_scheduler() == 2);
    ReadyQueue.queue_size = 0;
    assert(scheduler() == OS_NO_TASK);
}
static void test_mutex(void)
{
    mutex_t first, second;
    setup(); mutex_init(&first); mutex_init(&second);
    running(1); assert(mutex_lock(&first)); assert(mutex_lock(&second));
    assert(!mutex_lock(&first));
    running(2); assert(!mutex_unlock(&first));
    block_on(&first, 4);
    block_on(&second, 5);
    assert(ReadyQueue.tasks[1].task_prior == 5);
    running(1); assert(mutex_unlock(&second));
    assert(second.owner == 5 && ReadyQueue.tasks[5].task_state == READY);
    assert(ReadyQueue.tasks[1].task_prior == 4);
    assert(mutex_unlock(&first));
    assert(first.owner == 4 && ReadyQueue.tasks[1].task_prior == 1);
    running(4); assert(mutex_unlock(&first));
    assert(first.owner == OS_NO_TASK && !mutex_unlock(&first));

    /* Cadeia alta -> media -> baixa e rejeicao de ciclo. */
    setup(); mutex_init(&first); mutex_init(&second);
    running(1); assert(mutex_lock(&first));
    running(2); assert(mutex_lock(&second));
    block_on(&first, 2); block_on(&second, 5);
    assert(ReadyQueue.tasks[1].task_prior == 5 && ReadyQueue.tasks[2].task_prior == 5);
    running(1); assert(!mutex_lock(&second)); assert(mutex_unlock(&first));
    assert(ReadyQueue.tasks[1].task_prior == 1 && ReadyQueue.tasks[2].task_prior == 5);
    running(2); assert(mutex_unlock(&second));
    assert(ReadyQueue.tasks[2].task_prior == 2);

    /* Maior prioridade primeiro; FIFO no empate. */
    setup(); mutex_init(&first); running(1); assert(mutex_lock(&first));
    ReadyQueue.tasks[3].base_prior = 4;
    block_on(&first, 3); block_on(&first, 4); block_on(&first, 5);
    running(1); assert(mutex_unlock(&first)); assert(first.owner == 5);
    running(5); assert(mutex_unlock(&first)); assert(first.owner == 3);
    running(3); assert(mutex_unlock(&first)); assert(first.owner == 4);
}
static void test_memory(void)
{
    setup();
    assert(os_malloc(0) == NULL && os_malloc(UINT16_MAX) == NULL);
    void *a = os_malloc(120), *b = os_malloc(120), *c = os_malloc(120);
    assert(a && b && c && a != b);
    assert((uintptr_t)a % _Alignof(block_info_t) == 0);
    memset(c, 0x5a, 120);
    os_free(a); os_free(b);
    void *joined = os_malloc(240); assert(joined == a);
    assert(((unsigned char *)c)[119] == 0x5a);
    os_free(joined); os_free(c);
    void *full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full == a);
    assert(os_malloc(1) == NULL);
    int external;
    os_free(&external); os_free((char *)full + 1); os_free(NULL);
    assert(os_malloc(1) == NULL);
    os_free(full);
    /* Ordem inversa tambem recupera o heap inteiro. */
    a = os_malloc(1); b = os_malloc(3); c = os_malloc(5);
    os_free(c); os_free(b); os_free(a);
    full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full); os_free(full);
    void *blocks[128]; unsigned count = 0;
    while (count < 128 && (blocks[count] = os_malloc(7)) != NULL) ++count;
    assert(count > 1 && count < 128);
    for (unsigned i = 0; i < count; i += 2) os_free(blocks[i]);
    for (unsigned i = 1; i < count; i += 2) os_free(blocks[i]);
    full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full); os_free(full);
}
static sem_t semaphore;
static void release_semaphore(void)
{
    assert(ReadyQueue.tasks[5].task_state == BLOCKED);
    running(1); sem_post(&semaphore);
    assert(ReadyQueue.tasks[5].task_state == READY);
    running(5);
}
static void test_semaphore_and_ticks(void)
{
    setup(); sem_init(&semaphore, 0); running(5);
    resume_hook = release_semaphore;
    sem_wait(&semaphore);
    assert(semaphore.sem_count == 0 && !masked);
    ReadyQueue.tasks[5].task_state = BLOCKED;
    _T1Interrupt();
    assert(ReadyQueue.tasks[5].task_state == BLOCKED);
    running(4); ReadyQueue.tasks[4].task_delay = 2; ReadyQueue.tasks[4].task_state = WAITING;
    _T1Interrupt(); assert(ReadyQueue.tasks[4].task_state == WAITING);
    _T1Interrupt(); assert(ReadyQueue.tasks[4].task_state == RUNNING);
    running(3); _INT0Interrupt(); assert(ReadyQueue.tasks[3].task_state == READY);
    irq_state_t old = os_irq_save();
    sem_post(&semaphore); assert(masked); os_irq_restore(old); assert(!masked);
    running(4); resume_hook = _INT0Interrupt;
    task_delay(0); assert(ReadyQueue.tasks[4].task_state == RUNNING);
    setup(); running(5); delayed = true;
    task_yield();
    assert(!delayed && ReadyQueue.tasks[5].task_state == RUNNING);
}
static m_queue_t queue;
static void produce_message(void)
{
    assert(ReadyQueue.tasks[5].task_state == BLOCKED);
    running(1); m_queue_write(&queue, 'x'); running(5);
}
static void consume_message(void)
{
    char message;
    assert(ReadyQueue.tasks[5].task_state == BLOCKED);
    running(1); m_queue_read(&queue, &message); assert(message == 'a'); running(5);
}
static void test_messages(void)
{
    setup(); memset(&queue, 0, sizeof queue);
    void *full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full);
    assert(!m_queue_init(&queue)); os_free(full);
    assert(m_queue_init(&queue) && !m_queue_init(&queue));
    running(5); resume_hook = produce_message;
    char message; m_queue_read(&queue, &message); assert(message == 'x');
    for (char i = 0; i < SIZE_MESSAGE_QUEUE; ++i) m_queue_write(&queue, 'a' + i);
    assert(!m_queue_destroy(&queue));
    resume_hook = consume_message; m_queue_write(&queue, 'z');
    for (char i = 1; i < SIZE_MESSAGE_QUEUE; ++i) {
        m_queue_read(&queue, &message); assert(message == 'a' + i);
    }
    m_queue_read(&queue, &message); assert(message == 'z');
    for (unsigned i = 0; i < 30; ++i) {
        m_queue_write(&queue, (char)i); m_queue_read(&queue, &message); assert(message == (char)i);
    }
    assert(m_queue_destroy(&queue) && !m_queue_destroy(&queue));
    full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full); os_free(full);
}
int main(void)
{
    test_scheduler(); puts("OK: prioridade, Round-Robin, idle e limites");
    test_mutex(); puts("OK: mutex, posse, heranca multipla e transitiva, restauracao e FIFO");
    test_memory(); puts("OK: alinhamento, exaustao, coalescencia e integridade do heap");
    test_semaphore_and_ticks(); puts("OK: semaforos, secoes aninhadas e preservacao dos bloqueios");
    test_messages(); puts("OK: fila dinamica, FIFO circular e espera por dados/espaco");
    return 0;
}
