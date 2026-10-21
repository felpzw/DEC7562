#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
typedef uint16_t irq_state_t;

#ifdef OS_HOST_TEST
/* O host simula interrupcoes; a logica do RTOS e a mesma do firmware. */
irq_state_t os_irq_save(void);
void os_irq_restore(irq_state_t state);
void os_host_context_switch(void);
#define OS_INTERRUPT
#else
#include <xc.h>
#define OS_INTERRUPT __attribute__((interrupt, no_auto_psv, naked))
static inline irq_state_t os_irq_save(void)
{
    irq_state_t state = SR;
    __builtin_disable_interrupts();
    asm volatile ("" ::: "memory");
    return state;
}
static inline void os_irq_restore(irq_state_t state)
{
    asm volatile ("" ::: "memory");
    SR = state;
}
#endif
#endif
