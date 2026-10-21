#include "adc.h"
#include "sync.h"
#include <xc.h>
#include <stddef.h>

static mutex_t adc_mutex;
static bool initialized;

bool adc_init(uint8_t channel)
{
    if (channel > 15) return false;
    AD1CON1 = 0;
    AD1CON2 = 0; /* referencias AVDD/AVSS, um resultado por conversao. */
    AD1CON3 = 0;
    AD1CHS = 0;
    AD1CSSL = 0;
    AD1PCFG = (uint16_t)~(1U << channel);
    TRISB |= (uint16_t)(1U << channel);
    AD1CHSbits.CH0SA = channel;
    AD1CON1bits.FORM = 0;
    AD1CON1bits.SSRC = 7; /* contador interno encerra a amostragem. */
    AD1CON3bits.ADCS = 3; /* TAD = 4 * TCY = 1 us em FCY=4 MHz. */
    AD1CON3bits.SAMC = 16;
    IEC0bits.AD1IE = 0;
    IFS0bits.AD1IF = 0;
    mutex_init(&adc_mutex);
    initialized = true;
    AD1CON1bits.ADON = 1;
    return true;
}

bool adc_read(uint16_t *sample)
{
    uint16_t timeout = UINT16_MAX;
    if (!initialized || sample == NULL || !mutex_lock(&adc_mutex)) return false;
    AD1CON1bits.DONE = 0;
    AD1CON1bits.SAMP = 1;
    while (!AD1CON1bits.DONE && --timeout != 0) Nop();
    bool success = AD1CON1bits.DONE != 0;
    if (success) *sample = ADC1BUF0 & ADC_MAX_VALUE;
    else AD1CON1bits.SAMP = 0;
    mutex_unlock(&adc_mutex);
    return success;
}
