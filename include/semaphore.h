#ifndef YUAN_RTOS_SEMAPHORE_H
#define YUAN_RTOS_SEMAPHORE_H

#include "yr_config.h"

#if YR_SUPPORT_SEMAPHORE

#include "ipc.h"
#include "portable.h"
#include "yr_def.h"

typedef struct yr_semaphore_t {
    yr_ipc_base_t ipc_base;
    yr_uint16_t max_count; 
    yr_uint16_t current_count;
} yr_semaphore_t;

yr_err_t yr_semaphore_init( yr_semaphore_t* sem, yr_uint16_t max_count,yr_uint16_t init_count, yr_uint32_t flag);
yr_err_t yr_semaphore_delete( yr_semaphore_t* sem);
yr_err_t yr_semaphore_take( yr_semaphore_t* sem, yr_uint32_t wait_ticks);
yr_err_t yr_semaphore_give( yr_semaphore_t* sem);

yr_err_t yr_semaphore_take_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);
yr_err_t yr_semaphore_give_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);

#endif /* YR_SUPPORT_SEMAPHORE */

#endif /* YUAN_RTOS_SEMAPHORE_H */
