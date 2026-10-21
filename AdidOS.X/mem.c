#include "mem.h"
#include "types.h"
#include "platform.h"
#include <stddef.h>

static union {
    block_info_t alignment;
    uint8_t bytes[HEAP_SIZE];
} heap;
static bool initialized;

void os_heap_init(void)
{
    block_info_t *first = (block_info_t *)heap.bytes;
    first->data.size = (HEAP_SIZE / sizeof(block_info_t) - 1) * sizeof(block_info_t);
    first->data.free = 1;
    initialized = true;
}

void *os_malloc(uint16_t nr_bytes)
{
    uint16_t offset = 0;
    uint16_t aligned;
    irq_state_t state;
    void *result = NULL;
    if (!initialized || nr_bytes == 0 || nr_bytes > HEAP_SIZE - sizeof(block_info_t))
        return NULL;
    aligned = ((nr_bytes + sizeof(block_info_t) - 1) / sizeof(block_info_t)) * sizeof(block_info_t);
    state = os_irq_save();
    while (offset + sizeof(block_info_t) <= HEAP_SIZE) {
        block_info_t *block = (block_info_t *)(heap.bytes + offset);
        if (block->data.free && block->data.size >= aligned) {
            uint16_t remaining = block->data.size - aligned;
            if (remaining >= 2 * sizeof(block_info_t)) {
                block_info_t *next = (block_info_t *)(heap.bytes + offset + sizeof(block_info_t) + aligned);
                next->data.size = remaining - sizeof(block_info_t);
                next->data.free = 1;
                block->data.size = aligned;
            }
            block->data.free = 0;
            result = heap.bytes + offset + sizeof(block_info_t);
            break;
        }
        offset += sizeof(block_info_t) + block->data.size;
    }
    os_irq_restore(state);
    return result;
}

void os_free(void *address)
{
    uint16_t offset = 0;
    irq_state_t state;
    if (!initialized || address == NULL) return;
    state = os_irq_save();
    /* Nao interpreta enderecos externos ou interiores como cabecalhos. */
    while (offset + sizeof(block_info_t) <= HEAP_SIZE) {
        block_info_t *block = (block_info_t *)(heap.bytes + offset);
        if (heap.bytes + offset + sizeof(block_info_t) == address) {
            block->data.free = 1;
            break;
        }
        offset += sizeof(block_info_t) + block->data.size;
    }
    /* Une o anterior e todos os posteriores, sem atravessar blocos ocupados. */
    offset = 0;
    while (offset + sizeof(block_info_t) <= HEAP_SIZE) {
        block_info_t *block = (block_info_t *)(heap.bytes + offset);
        uint16_t next_offset = offset + sizeof(block_info_t) + block->data.size;
        if (next_offset + sizeof(block_info_t) > HEAP_SIZE) break;
        block_info_t *next = (block_info_t *)(heap.bytes + next_offset);
        if (block->data.free && next->data.free)
            block->data.size += sizeof(block_info_t) + next->data.size;
        else
            offset = next_offset;
    }
    os_irq_restore(state);
}
