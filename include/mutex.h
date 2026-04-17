#ifndef YUAN_RTOS_MUTEX_H
#define YUAN_RTOS_MUTEX_H

#include "yr_config.h"

#if YR_SUPPORT_MUTEX

#include "ipc.h"
#include "portable.h"
#include "task.h"

#define YR_MUTEX_HOLD_MAX       (yr_uint8_t)(0xFFU)
#define YR_MUTEX_PRIO_INVALID   (yr_uint8_t)(0xFFU)

typedef struct yr_mutex_t {
    yr_ipc_base_t ipc_base;
    yr_task_t *owner;
    yr_uint8_t hold;    /* 递归锁持有的深度 */
    yr_uint8_t original_priority;
} yr_mutex_t;

yr_err_t yr_mutex_init( yr_mutex_t* mutex, yr_uint32_t flag);
yr_err_t yr_mutex_delete( yr_mutex_t* mutex);
yr_err_t yr_mutex_take( yr_mutex_t* mutex, yr_uint32_t wait_ticks);
yr_err_t yr_mutex_give( yr_mutex_t* mutex);

#endif /* YR_SUPPORT_MUTEX */

#endif /* YUAN_RTOS_MUTEX_H */
