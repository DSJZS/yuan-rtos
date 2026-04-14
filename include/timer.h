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

yr_uint32_t yr_get_current_ticks(void);

/* 定时器初始化 */
void yr_timer_list_init(void);
yr_err_t yr_timer_init( yr_timer_t *timer, yr_timer_func_t func, void *param, yr_uint32_t ticks);
yr_err_t yr_timer_start( yr_timer_t *timer);
yr_err_t yr_timer_stop( yr_timer_t *timer);
yr_err_t yr_timer_set_ticks( yr_timer_t *timer, yr_uint32_t ticks);

/* 每 tick 都要执行的回调函数 */
void yr_tick_update(void);
void yr_timeout_default_func(void *param);

#endif /* YUAN_RTOS_TIMER_H */
