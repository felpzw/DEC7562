#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include <stdint.h>

#define SCHEDULER_TRACE_SIZE 32
typedef struct scheduler_trace_entry {
    uint16_t tick;
    uint8_t task_id;
    uint8_t priority;
} scheduler_trace_entry_t;
extern volatile scheduler_trace_entry_t scheduler_trace[SCHEDULER_TRACE_SIZE];
/* Proxima posicao de escrita; registros anteriores formam um anel. */
extern volatile uint8_t scheduler_trace_position;

uint8_t scheduler(void);
uint8_t rr_scheduler(void);
uint8_t prior_scheduler(void);


#endif	/* SCHEDULER_H */

