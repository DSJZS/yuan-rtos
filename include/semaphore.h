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

/**
 * @brief 初始化信号量。
 * @param sem 信号量对象指针。
 * @param max_count 最大计数值。
 * @param init_count 初始计数值。
 * @param flag 阻塞任务的排队策略。
 * @return 初始化结果。
 */
yr_err_t yr_semaphore_init( yr_semaphore_t* sem, yr_uint16_t max_count,yr_uint16_t init_count, yr_uint32_t flag);

/**
 * @brief 删除信号量并唤醒等待任务。
 * @param sem 信号量对象指针。
 * @return 操作结果。
 */
yr_err_t yr_semaphore_delete( yr_semaphore_t* sem);

/**
 * @brief 获取信号量。
 * @param sem 信号量对象指针。
 * @param wait_ticks 获取失败时允许等待的 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_semaphore_take( yr_semaphore_t* sem, yr_uint32_t wait_ticks);

/**
 * @brief 释放信号量。
 * @param sem 信号量对象指针。
 * @return 操作结果。
 */
yr_err_t yr_semaphore_give( yr_semaphore_t* sem);

/**
 * @brief 在中断中获取信号量。
 * @param sem 信号量对象指针。
 * @param need_switch 若需要切换任务则置为 YR_TRUE。
 * @return 操作结果。
 */
yr_err_t yr_semaphore_take_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);

/**
 * @brief 在中断中释放信号量。
 * @param sem 信号量对象指针。
 * @param need_switch 若需要切换任务则置为 YR_TRUE。
 * @return 操作结果。
 */
yr_err_t yr_semaphore_give_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);

#endif /* YR_SUPPORT_SEMAPHORE */

#endif /* YUAN_RTOS_SEMAPHORE_H */
