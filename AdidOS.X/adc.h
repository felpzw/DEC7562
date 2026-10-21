#ifndef ADC_H
#define ADC_H
#include <stdbool.h>
#include <stdint.h>
#define ADC_MAX_VALUE 1023U
/* ADC1, canal AN0..AN15, AVDD/AVSS. Inicializar antes do escalonador. */
bool adc_init(uint8_t channel);
/* Leitura sincrona serializada por mutex; false em erro ou timeout. */
bool adc_read(uint16_t *sample);
#endif
