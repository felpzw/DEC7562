#ifndef PWM_H
#define PWM_H
#include "os_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct pwm_driver {
    uint16_t periodo;
    uint16_t ciclo;
    uint8_t prescaler;
    bool configured;
    bool status;
} pwm_driver_t;
/* Uma instancia controla OC1/RD0 e Timer2; configurar antes de iniciar. */
bool pwm_config(pwm_driver_t *pwm, uint16_t frequencia);
void pwm_start(pwm_driver_t *pwm);
void pwm_stop(pwm_driver_t *pwm);
/* Percentual 0..100; permitido tambem antes de start. */
void pwm_set_duty(pwm_driver_t *pwm, uint16_t duty);
#endif
