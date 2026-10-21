#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "message.h"
#include "mem.h"
#include "adc.h"
#include "pwm.h"
#include "uart.h"
#include <xc.h>
#include <stddef.h>

static m_queue_t samples;
static sem_t sensor_start, monitor_start;
static mutex_t uart_mutex;
static pwm_driver_t pwm;
volatile demo_stats_t demo_stats;

/* Trabalho deliberadamente ativo para permitir preempcao e observar RR/PI.
 * As esperas normais da aplicacao usam task_delay ou objetos bloqueantes. */
static void work_for_ticks(uint16_t duration, bool monitor)
{
    uint32_t start = os_get_ticks(), last = start;
    while ((uint32_t)(os_get_ticks() - start) < duration) {
        uint32_t now = os_get_ticks();
        if (now != last) {
            last = now;
            if (monitor) { ++demo_stats.monitor_slices; LATEbits.LATE2 ^= 1; }
            else { ++demo_stats.pwm_slices; LATEbits.LATE1 ^= 1; }
        }
        Nop();
    }
}

static bool validate_heap(void)
{
    const uint16_t guard_size = HEAP_SIZE - 3 * 96 - 4 * sizeof(block_info_t);
    void *left = os_malloc(96), *middle = os_malloc(96);
    uint8_t *guard = os_malloc(guard_size);
    bool success = false;
    if (left != NULL && middle != NULL && guard != NULL) {
        guard[0] = 0xa5;
        guard[guard_size - 1] = 0x5a;
        os_free(left); left = NULL;
        os_free(middle); middle = NULL;
        /* A cauda e menor que 192 bytes: exige a juncao dos dois primeiros. */
        void *joined = os_malloc(192);
        success = joined != NULL && guard[0] == 0xa5 && guard[guard_size - 1] == 0x5a;
        os_free(joined);
    }
    os_free(left); os_free(middle); os_free(guard);
    return success;
}

bool config_user_app(void)
{
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE2 = 0;
    TRISFbits.TRISF3 = 0;
    LATEbits.LATE0 = LATEbits.LATE1 = LATEbits.LATE2 = 0;
    UART_init(9600);
    mutex_init(&uart_mutex);
    sem_init(&sensor_start, 0);
    sem_init(&monitor_start, 0);
    demo_stats.heap_coalesced = validate_heap();
    if (!demo_stats.heap_coalesced || !m_queue_init(&samples)) return false;
    if (!adc_init(0) || !pwm_config(&pwm, 1000)) {
        m_queue_destroy(&samples);
        return false;
    }
    pwm_set_duty(&pwm, 0);
    pwm_start(&pwm);
    return true;
}

TASK tarefa_adc(void)
{
    /* A baixa prioridade libera esta tarefa enquanto possui o mutex. */
    sem_wait(&sensor_start);
    if (!mutex_lock(&uart_mutex)) while (1) Nop();
    UART_write_new_line("[PI] alta adquiriu UART apos desbloqueio");
    mutex_unlock(&uart_mutex);
    while (1) {
        uint16_t sample;
        if (adc_read(&sample)) {
            uint16_t duty = (uint32_t)sample * 100 / ADC_MAX_VALUE;
            demo_stats.adc_sample = sample;
            m_queue_write(&samples, (char)duty);
            LATEbits.LATE0 ^= 1;
            if (mutex_lock(&uart_mutex)) {
                UART_write_string("ADC="); UART_write_int(sample);
                UART_write_string(" PWM="); UART_write_int(duty);
                UART_write_new_line("%");
                mutex_unlock(&uart_mutex);
            }
        } else {
            ++demo_stats.adc_errors;
        }
        task_delay(25); /* 100 ms, alem do tempo de execucao. */
    }
}

TASK tarefa_pwm(void)
{
    while (1) {
        char duty;
        m_queue_read(&samples, &duty);
        pwm_set_duty(&pwm, (uint8_t)duty);
        demo_stats.pwm_duty = (uint8_t)duty;
        ++demo_stats.messages;
        /* As duas tarefas de prioridade 2 ficam prontas ao mesmo tempo. */
        sem_post(&monitor_start);
        work_for_ticks(4, false);
    }
}

TASK tarefa_monitor(void)
{
    while (1) {
        sem_wait(&monitor_start);
        work_for_ticks(4, true);
    }
}

TASK tarefa_diagnostico(void)
{
    if (!mutex_lock(&uart_mutex)) while (1) Nop();
    UART_write_new_line("[PI] baixa possui UART; libera tarefa alta");
    sem_post(&sensor_start);
    /* A alta tenta adquirir UART e bloqueia. A baixa deve herdar 4. */
    uint32_t start = os_get_ticks();
    while ((uint32_t)(os_get_ticks() - start) < 5) Nop();
    demo_stats.inherited_priority = task_get_priority();
    UART_write_string("[PI] prioridade herdada=");
    UART_write_int(demo_stats.inherited_priority);
    UART_write_new_line("");
    mutex_unlock(&uart_mutex);
    demo_stats.restored_priority = task_get_priority();
    while (1) {
        if (mutex_lock(&uart_mutex)) {
            UART_write_string("[RTOS] heap=OK prioridade restaurada=");
            UART_write_int(demo_stats.restored_priority);
            UART_write_string(" RR PWM/monitor=");
            UART_write_int(demo_stats.pwm_slices);
            UART_write_char('/'); UART_write_int(demo_stats.monitor_slices);
            UART_write_new_line("");
            mutex_unlock(&uart_mutex);
        }
        task_delay(100);
    }
}
