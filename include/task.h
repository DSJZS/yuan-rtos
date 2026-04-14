#ifndef YUAN_RTOS_TASK_H
#define YUAN_RTOS_TASK_H

#include "portable.h"
#include "yr_def.h"
#include "list.h"

/* 任务优先级范围： [0 ,YR_TASK_PRIORITY_MAX) ，类似中断优先级，越小优先级越大 */
#define YR_TASK_MAX_PRIORITY    (32)

typedef struct yr_task_t {
    yr_list_t list_node;

    void *sp;
    void *entry;
    // void* exit;
    void *param;
    void *stack_addr;
    yr_uint32_t stack_size;

    yr_uint8_t init_priority;
    yr_uint8_t current_priority;
    yr_uint32_t priority_mask; /* 永远等于 1 << current_priority */

    yr_uint32_t init_tick;
    yr_uint32_t remaining_tick;
    
    yr_uint32_t status;
} yr_task_t;

typedef void (*yr_task_func_t)(void *param);

/* 初始化任务 */
yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t tick);
/* 初始化任务，与 yr_task_init 的区别是使用默认时间片长度，建议没有特殊需求的话使用这个函数初始化任务 */
yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority);

/* 将任务交由调度器管理 */
yr_err_t yr_task_start( yr_task_t *task);

#endif /* YUAN_RTOS_TASK_H */
