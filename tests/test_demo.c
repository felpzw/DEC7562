#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include "kernel.h"
#include "user_app.h"
#include "scheduler.h"
#include "mem.h"
#include <xc.h>

extern queue_t ReadyQueue;
volatile timer_reg_t test_t1, test_t2;
volatile flags_reg_t test_ifs;
volatile enable_reg_t test_iec;
volatile priority_reg_t test_ipc;
volatile oc_reg_t test_oc;
volatile adc1_reg_t test_adc1;
volatile adc3_reg_t test_adc3;
volatile adchs_reg_t test_adchs;
volatile trisd_reg_t test_trisd;
volatile latd_reg_t test_latd;
volatile trise_reg_t test_trise;
volatile late_reg_t test_late;
volatile trisf_reg_t test_trisf;
volatile uartmode_reg_t test_uartmode;
volatile uartsta_reg_t test_uartsta;
volatile uint16_t OC1R, OC1RS, AD1CON2, AD1CSSL, AD1PCFG, TRISB, ADC1BUF0;
volatile uint16_t U1BRG, U1TXREG, TMR1, PR1, TMR2, PR2, TBLPAG, PSVPAG, CORCON;
clkdiv_reg_t CLKDIVbits;
static irq_state_t masked;
irq_state_t os_irq_save(void) { irq_state_t old = masked; masked = 1; return old; }
void os_irq_restore(irq_state_t state) { masked = state; }
void os_host_context_switch(void) { assert(!"despacho inesperado durante o boot"); }
void os_host_nop(void) {}

int main(void)
{
    os_heap_init();
    void *full = os_malloc(HEAP_SIZE - sizeof(block_info_t)); assert(full);
    assert(!config_user_app()); /* inicializacao informa falha de memoria. */
    os_free(full);
    assert(os_config());
    assert(masked); /* boot permanece atomico ate restaurar o primeiro contexto. */
    assert(demo_stats.heap_coalesced);
    assert(ReadyQueue.queue_size == 1 && ReadyQueue.tasks[0].task_func == idle);
    assert(create_task(tarefa_adc, SENSOR_PRIORITY));
    assert(create_task(tarefa_pwm, WORKER_PRIORITY));
    assert(create_task(tarefa_monitor, WORKER_PRIORITY));
    assert(create_task(tarefa_diagnostico, LOGGER_PRIORITY));
    assert(ReadyQueue.queue_size == 5 && scheduler() == 1);
    assert(ReadyQueue.tasks[2].task_prior == ReadyQueue.tasks[3].task_prior);
    assert(PR1 == 1999 && PR2 == 3999 && OC1RS == 0 && T2CONbits.TON);
    assert(U1BRG == 25 && U1MODEbits.UARTEN && U1STAbits.UTXEN);
    assert(AD1CON1bits.ADON && AD1PCFG == 0xfffe);
    puts("OK: boot integrado, falha de memoria, quatro tarefas e configuracao dos perifericos");
    return 0;
}
