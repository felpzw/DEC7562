#ifndef TYPES_H
#define	TYPES_H

#include "os_config.h"
#include <stdint.h>
#include <stdbool.h>

//#define IDLE 1

typedef void (*callback)(void);

typedef void TASK;

typedef enum {READY = 0, RUNNING, WAITING, BLOCKED} state_t;

struct mutex;

typedef struct tcb {
    uint8_t task_id;
    uint8_t task_prior;
    uint8_t base_prior;
    uint16_t task_delay;
    callback task_func;
    state_t task_state;
    uint16_t task_stack[MAX_STACK_SIZE];
    uint16_t *sp;
    struct mutex *waiting_mutex;
} tcb_t;

typedef struct queue {
    tcb_t tasks[MAX_TASKS+1];
    uint8_t queue_size;
} queue_t;

typedef struct sem
{
    uint8_t sem_queue[MAX_TASKS];
    int sem_count;
    uint8_t s_queue_input;
    uint8_t s_queue_output;
} sem_t;

typedef struct mutex {
    uint8_t owner;
    uint8_t waiters[MAX_TASKS];
    uint8_t waiter_count;
} mutex_t;

typedef struct m_queue {
    char *queue;
    uint8_t pos_write;
    uint8_t pos_read;
    sem_t s_write;
    sem_t s_read;
} m_queue_t;

typedef union block_info {
    struct {
        uint16_t size;
        uint8_t free;
    } data;
    uint32_t align_integer;
    void *align_pointer;
    callback align_function;
} block_info_t;

#endif	/* TYPES_H */

