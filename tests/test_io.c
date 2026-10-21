#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "adc.h"
#include "pwm.h"
#include "sync.h"
#include <xc.h>

queue_t ReadyQueue;
uint8_t task_running = 1;
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
volatile uint16_t OC1R, OC1RS, AD1CON2, AD1CSSL, AD1PCFG, TRISB, ADC1BUF0, TMR2, PR2;
static irq_state_t masked;
static bool conversion_ok = true;
irq_state_t os_irq_save(void) { irq_state_t old = masked; masked = 1; return old; }
void os_irq_restore(irq_state_t state) { masked = state; }
void os_host_context_switch(void) { assert(masked); }
void os_host_nop(void) { if (conversion_ok && AD1CON1bits.SAMP) AD1CON1bits.DONE = 1; }

int main(void)
{
    pwm_driver_t pwm = {0};
    ReadyQueue.queue_size = 2;
    ReadyQueue.tasks[1].base_prior = 1;
    ReadyQueue.tasks[1].task_prior = 1;
    uint16_t sample = 99;
    assert(!adc_read(&sample));
    assert(!adc_init(16));
    assert(adc_init(0));
    assert(AD1PCFG == 0xfffe && (TRISB & 1));
    assert(AD1CON1bits.ADON && AD1CON1bits.SSRC == 7);
    assert(AD1CON3bits.SAMC == 16 && AD1CON3bits.ADCS == 3);
    ADC1BUF0 = 512; assert(adc_read(&sample) && sample == 512);
    assert(!adc_read(NULL));
    conversion_ok = false; assert(!adc_read(&sample) && sample == 512);
    conversion_ok = true; ADC1BUF0 = 1023;
    assert(adc_read(&sample) && sample == 1023); /* mutex liberado no timeout. */
    assert(!pwm_config(NULL, 1000) && !pwm_config(&pwm, 0));
    pwm_start(&pwm); assert(!pwm.status);
    assert(pwm_config(&pwm, 1000) && pwm.periodo == 3999 && pwm.prescaler == 0);
    assert(TRISDbits.TRISD0 == 0);
    pwm_set_duty(&pwm, 50); assert(pwm.ciclo == 2000);
    pwm_start(&pwm); assert(pwm.status && OC1R == 2000 && OC1RS == 2000);
    assert(T2CONbits.TON && OC1CONbits.OCM == 6 && PR2 == 3999);
    pwm_set_duty(&pwm, 0); assert(OC1RS == 0);
    pwm_set_duty(&pwm, 100); assert(OC1RS == 4000 && OC1RS > PR2);
    pwm_set_duty(&pwm, 101); assert(OC1RS == 4000);
    pwm_stop(&pwm); assert(!pwm.status && !T2CONbits.TON && !OC1CONbits.OCM);
    assert(pwm_config(&pwm, 1) && pwm.prescaler == 2 && pwm.periodo == 62499);
    pwm_set_duty(&pwm, 100); pwm_start(&pwm); assert(OC1RS == 62500);
    assert(!masked);
    puts("OK: ADC, timeout e recuperacao; PWM, prescaler e duty 0/50/100%");
    return 0;
}
