#ifndef MESSAGE_H
#define MESSAGE_H
#include "types.h"
/* Inicializar com {0}; destruir somente quando nenhum usuario estiver ativo. */
bool m_queue_init(m_queue_t *queue);
bool m_queue_destroy(m_queue_t *queue);
/* Fila inicializada obrigatoria; leitura/escrita podem bloquear. */
void m_queue_write(m_queue_t *queue, char msg);
void m_queue_read(m_queue_t *queue, char *msg);
#endif
