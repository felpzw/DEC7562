#include "kernel.h"
#include "user_app.h"
#include "uart.h"
#include <xc.h>

#ifndef OS_HOST_TEST
/* FRC de 8 MHz, FCY=4 MHz; divisor 1:1 configurado em UART_init. */
#pragma config FNOSC = FRC
#pragma config IESO = OFF
#pragma config POSCMOD = NONE
#pragma config OSCIOFNC = ON
#pragma config FWDTEN = OFF
#pragma config JTAGEN = OFF
#endif

int main(void)
{
    if (!os_config() ||
        !create_task(tarefa_adc, SENSOR_PRIORITY) ||
        !create_task(tarefa_pwm, WORKER_PRIORITY) ||
        !create_task(tarefa_monitor, WORKER_PRIORITY) ||
        !create_task(tarefa_diagnostico, LOGGER_PRIORITY)) {
        UART_write_new_line("ERRO: inicializacao do RTOS/aplicacao");
        LATEbits.LATE0 = LATEbits.LATE1 = LATEbits.LATE2 = 1;
        while (1) Nop();
    }
    os_start_scheduler();
    while (1) Nop();
    return 0;
}
