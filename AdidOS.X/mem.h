#ifndef MEM_H
#define MEM_H
#include <stdint.h>
/* Inicializar uma vez antes de criar objetos dinamicos. */
void os_heap_init(void);
void *os_malloc(uint16_t nr_bytes);
void os_free(void *address);
#endif
