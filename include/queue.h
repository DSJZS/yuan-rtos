#ifndef YUAN_RTOS_QUEUE_H
#define YUAN_RTOS_QUEUE_H

#include "ipc.h"
#include "portable.h"
#include "yr_def.h"

typedef struct yr_queue_t {
    yr_ipc_base_t send_ipc;
    yr_ipc_base_t receive_ipc;

    yr_uint8_t *buffer;
    yr_uint32_t capacity;
    yr_uint32_t item_size;

    yr_uint32_t item_count;
    yr_uint32_t head;
    yr_uint32_t tail;
} yr_queue_t;

yr_err_t yr_queue_init( yr_queue_t *queue, yr_uint32_t item_size, yr_uint8_t *buffer, yr_uint32_t buffer_size, yr_uint32_t flag);
yr_err_t yr_queue_delete( yr_queue_t *queue);
yr_err_t yr_queue_reset( yr_queue_t *queue);
yr_err_t yr_queue_send( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);
yr_err_t yr_queue_receive( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);

#endif /* YUAN_RTOS_QUEUE_H */
