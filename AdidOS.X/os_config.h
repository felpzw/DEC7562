#ifndef OS_CONFIG_H
#define	OS_CONFIG_H

#define FCY                 4000000UL

#define MAX_TASKS           5
#define MAX_STACK_SIZE      128
#define SIZE_MESSAGE_QUEUE  4
#define HEAP_SIZE           1024
#define OS_TICK_MS          4
#define OS_NO_TASK          UINT8_MAX

#define RR_SCHEDULER        1
#define PRIOR_SCHEDULER     2

#ifndef DEFAULT_SCHEDULER
#define DEFAULT_SCHEDULER   PRIOR_SCHEDULER
#endif

#endif	/* OS_CONFIG_H */

