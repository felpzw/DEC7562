#ifndef USER_APP_H
#define USER_APP_H
#include "types.h"

#define SENSOR_PRIORITY 4
#define WORKER_PRIORITY 2
#define LOGGER_PRIORITY 1

typedef struct demo_stats {
    uint16_t adc_sample;
    uint16_t pwm_duty;
    uint16_t messages;
    uint16_t pwm_slices;
    uint16_t monitor_slices;
    uint16_t adc_errors;
    uint8_t inherited_priority;
    uint8_t restored_priority;
    bool heap_coalesced;
} demo_stats_t;
extern volatile demo_stats_t demo_stats;

TASK tarefa_adc(void);
TASK tarefa_pwm(void);
TASK tarefa_monitor(void);
TASK tarefa_diagnostico(void);
bool config_user_app(void);
#endif
