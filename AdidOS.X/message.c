#include "message.h"
#include "sync.h"
#include "mem.h"
#include "platform.h"
#include <stddef.h>

bool m_queue_init(m_queue_t *queue)
{
    if (queue == NULL || queue->queue != NULL) return false;
    queue->queue = os_malloc(SIZE_MESSAGE_QUEUE);
    if (queue->queue == NULL) return false;
    queue->pos_read = 0;
    queue->pos_write = 0;
    sem_init(&queue->s_write, SIZE_MESSAGE_QUEUE);
    sem_init(&queue->s_read, 0);
    return true;
}

bool m_queue_destroy(m_queue_t *queue)
{
    irq_state_t state = os_irq_save();
    if (queue == NULL || queue->queue == NULL ||
        queue->s_write.sem_count != SIZE_MESSAGE_QUEUE || queue->s_read.sem_count != 0) {
        os_irq_restore(state);
        return false;
    }
    os_free(queue->queue);
    queue->queue = NULL;
    os_irq_restore(state);
    return true;
}

void m_queue_write(m_queue_t *queue, char msg)
{
    sem_wait(&queue->s_write);
    irq_state_t state = os_irq_save();
    queue->queue[queue->pos_write] = msg;
    queue->pos_write = (queue->pos_write + 1) % SIZE_MESSAGE_QUEUE;
    sem_post(&queue->s_read);
    os_irq_restore(state);
}

void m_queue_read(m_queue_t *queue, char *msg)
{
    sem_wait(&queue->s_read);
    irq_state_t state = os_irq_save();
    *msg = queue->queue[queue->pos_read];
    queue->pos_read = (queue->pos_read + 1) % SIZE_MESSAGE_QUEUE;
    sem_post(&queue->s_write);
    os_irq_restore(state);
}
