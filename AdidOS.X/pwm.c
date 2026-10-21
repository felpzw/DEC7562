#include "pwm.h"
#include "platform.h"
#include <xc.h>
#include <stddef.h>

bool pwm_config(pwm_driver_t *pwm, uint16_t frequencia)
{
    static const uint16_t divisors[] = {1, 8, 64, 256};
    uint8_t prescaler;
    uint32_t ticks = 0;
    if (pwm == NULL || frequencia == 0) return false;
    for (prescaler = 0; prescaler < 4; ++prescaler) {
        ticks = FCY / ((uint32_t)divisors[prescaler] * frequencia);
        /* PR2+1 deve caber em OC1RS para representar tambem 100%. */
        if (ticks >= 2 && ticks <= UINT16_MAX) break;
    }
    if (prescaler == 4) return false;
    irq_state_t state = os_irq_save();
    pwm->periodo = (uint16_t)(ticks - 1);
    pwm->ciclo = 0;
    pwm->prescaler = prescaler;
    pwm->configured = true;
    pwm->status = false;
    OC1CON = 0;
    T2CON = 0;
    T2CONbits.TCKPS = prescaler;
    TRISDbits.TRISD0 = 0;
    LATDbits.LATD0 = 0;
    OC1R = 0;
    OC1RS = 0;
    TMR2 = 0;
    PR2 = pwm->periodo;
    os_irq_restore(state);
    return true;
}

void pwm_start(pwm_driver_t *pwm)
{
    irq_state_t state = os_irq_save();
    if (pwm != NULL && pwm->configured && !pwm->status) {
        TMR2 = 0;
        PR2 = pwm->periodo;
        OC1R = pwm->ciclo;
        OC1RS = pwm->ciclo;
        OC1CONbits.OCTSEL = 0;
        OC1CONbits.OCM = 6;
        T2CONbits.TON = 1;
        pwm->status = true;
    }
    os_irq_restore(state);
}

void pwm_stop(pwm_driver_t *pwm)
{
    irq_state_t state = os_irq_save();
    if (pwm != NULL && pwm->configured) {
        OC1CONbits.OCM = 0;
        T2CONbits.TON = 0;
        LATDbits.LATD0 = 0;
        pwm->status = false;
    }
    os_irq_restore(state);
}

void pwm_set_duty(pwm_driver_t *pwm, uint16_t duty)
{
    irq_state_t state = os_irq_save();
    if (pwm != NULL && pwm->configured) {
        if (duty > 100) duty = 100;
        pwm->ciclo = (uint16_t)(((uint32_t)pwm->periodo + 1) * duty / 100);
        if (pwm->status) OC1RS = pwm->ciclo;
    }
    os_irq_restore(state);
}
