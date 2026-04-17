#ifndef YUAN_RTOS_TASK_H
#define YUAN_RTOS_TASK_H

#include "portable.h"
#include "yr_def.h"
#include "list.h"
#include "timer.h"

/* 任务优先级范围： [0 ,YR_TASK_PRIORITY_MAX) ，类似中断优先级，越小优先级越大 */
#define YR_TASK_MAX_PRIORITY                (32)

typedef enum yr_task_status_t {
    /* 初始化 */
    YR_TASK_STATUS_INIT  = 0,
    /* 就绪 */
    YR_TASK_STATUS_READY,
    /* 运行 */
    YR_TASK_STATUS_RUNNING,
    /* 阻塞 */
    YR_TASK_STATUS_BLOCKED,
    /* 挂起(暂停) */
    YR_TASK_STATUS_SUSPENDED,
    /* 终止(待删除，僵尸任务) */
    YR_TASK_STATUS_TERMINATED,
    /* 删除(不再被管理) */
    YR_TASK_STATUS_DELETED,
}yr_task_status_t;

typedef enum yr_task_ctl_t {
    YR_TASK_CTL_GET_STATUS = 0,
    YR_TASK_CTL_GET_PRIORITY,
    YR_TASK_CTL_SET_PRIORITY,
    YR_TASK_CTL_CHECK
} yr_task_ctl_t;

typedef enum yr_task_block_reason_t {
    YR_TASK_BR_NONE = 0,
    YR_TASK_BR_SLEEP,
    YR_TASK_BR_IPC,
    YR_TASK_BR_CHECK,
} yr_task_block_reason_t;

typedef enum yr_task_block_notify_t {
    YR_TASK_BN_NONE = 0,
    YR_TASK_BN_WAIT_OK,
    YR_TASK_BN_WAIT_TIMEOUT,
    YR_TASK_BN_WAIT_IPC_DELETED,
    YR_TASK_BN_CHECK,
} yr_task_block_notify_t;

typedef struct yr_task_block_t {
    void *source;
    yr_uint8_t reason;
    yr_uint16_t notify;
} yr_task_block_t;

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

    yr_uint32_t init_ticks;
    yr_uint32_t remaining_ticks;
    
    yr_uint32_t status;
    
    yr_task_block_t block_info;
    yr_timer_t timer;

    yr_uint16_t async_notify;
} yr_task_t;

typedef void (*yr_task_func_t)(void *param);

/* 初始化任务 */
yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t ticks);
/* 初始化任务，与 yr_task_init 的区别是使用默认时间片长度，建议没有特殊需求的话使用这个函数初始化任务 */
yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority);

/* 将任务交由调度器管理 */
yr_err_t yr_task_start( yr_task_t *task);
void yr_task_sleep_ticks( yr_uint32_t ticks);
yr_err_t yr_task_sleep_until(yr_uint32_t *pre_ticks, yr_uint32_t inc_ticks);
yr_err_t yr_task_delete(yr_task_t *task);
void yr_task_cleanup_defunct(void);
yr_err_t yr_task_suspend( yr_task_t *task);

yr_err_t yr_task_ctrl( yr_task_t *task, yr_uint32_t cmd, void *arg, yr_bool_t *need_switch);
yr_err_t yr_task_set_priority( yr_task_t *task, yr_uint8_t priority);

yr_err_t yr_task_set_block_info( yr_task_t *task, void *source, yr_uint8_t reason, yr_uint16_t notify);

#endif /* YUAN_RTOS_TASK_H */
