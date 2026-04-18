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
} yr_mutex_t;

/**
 * @brief 初始化互斥锁。
 * @param mutex 互斥锁对象指针。
 * @param flag 阻塞任务的排队策略。
 * @return 初始化结果。
 */
yr_err_t yr_mutex_init( yr_mutex_t* mutex, yr_uint32_t flag);

/**
 * @brief 删除互斥锁并唤醒等待任务。
 * @param mutex 互斥锁对象指针。
 * @return 操作结果。
 */
yr_err_t yr_mutex_delete( yr_mutex_t* mutex);

/**
 * @brief 获取互斥锁。
 * @param mutex 互斥锁对象指针。
 * @param wait_ticks 获取失败时允许等待的 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_mutex_take( yr_mutex_t* mutex, yr_uint32_t wait_ticks);

/**
 * @brief 释放互斥锁。
 * @param mutex 互斥锁对象指针。
 * @return 操作结果。
 */
yr_err_t yr_mutex_give( yr_mutex_t* mutex);

#endif /* YR_SUPPORT_MUTEX */

#endif /* YUAN_RTOS_MUTEX_H */
