#ifndef YUAN_RTOS_TIMER_H
#define YUAN_RTOS_TIMER_H

#include "portable.h"
#include "list.h"
#include "yr_def.h"

typedef void (*yr_timer_func_t)(void *param);

typedef struct yr_timer_t {
    yr_list_t list_node;
    yr_timer_func_t timeout_func;
    void *param;
    yr_uint32_t init_ticks;
    yr_uint32_t timeout_ticks;
} yr_timer_t;

/**
 * @brief 获取当前系统 tick 计数。
 * @return 当前 tick 值。
 */
yr_uint32_t yr_get_current_ticks(void);

/**
 * @brief 初始化定时器链表。
 */
void yr_timer_list_init(void);

/**
 * @brief 初始化定时器对象。
 * @param timer 定时器对象指针。
 * @param func 超时回调函数。
 * @param param 传递给回调函数的参数。
 * @param ticks 定时器周期 tick 数。
 * @return 初始化结果。
 */
yr_err_t yr_timer_init( yr_timer_t *timer, yr_timer_func_t func, void *param, yr_uint32_t ticks);

/**
 * @brief 启动定时器。
 * @param timer 定时器对象指针。
 * @return 操作结果。
 */
yr_err_t yr_timer_start( yr_timer_t *timer);

/**
 * @brief 停止定时器。
 * @param timer 定时器对象指针。
 * @return 操作结果。
 */
yr_err_t yr_timer_stop( yr_timer_t *timer);

/**
 * @brief 设置定时器周期 tick 数。
 * @param timer 定时器对象指针。
 * @param ticks 新的周期 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_timer_set_ticks( yr_timer_t *timer, yr_uint32_t ticks);

/**
 * @brief 每个系统 tick 调用一次的更新函数。
 */
void yr_tick_update(void);

/**
 * @brief 默认的定时器超时回调函数。
 * @param param 超时关联参数。
 */
void yr_timeout_default_func(void *param);

#endif /* YUAN_RTOS_TIMER_H */
