#ifndef INT_H
#define	INT_H

#include "platform.h"

#ifdef OS_HOST_TEST
#define CONTEXT_SWITCH() os_host_context_switch()
#else
#define CONTEXT_SWITCH() (IFS0bits.INT0IF = 1)
#endif

void config_int0(void);
void OS_INTERRUPT _INT0Interrupt(void);

#endif	/* INT_H */

