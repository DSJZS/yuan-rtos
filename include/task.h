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

typedef enum yr_task_ctl_current_t {
    YR_TASK_CTL_GET_CUR_STATUS = 0,
    YR_TASK_CTL_GET_CUR_PRIORITY,
    YR_TASK_CTL_SET_CUR_PRIORITY,
    YR_TASK_CTL_CUR_CHECK
} yr_task_ctl_current_t;

typedef enum yr_task_msg_reason_t {
    YR_TASK_MR_NONE = 0,
    YR_TASK_MR_SLEEP,
    YR_TASK_MR_IPC,
    YR_TASK_MR_CHECK,
} yr_task_msg_reason_t;

typedef enum yr_task_msg_notify_t {
    YR_TASK_MN_NONE = 0,
    YR_TASK_MN_WAIT_OK,
    YR_TASK_MN_WAIT_TIMEOUT,
    YR_TASK_MN_WAIT_IPC_DELETED,
    YR_TASK_MN_CHECK,
} yr_task_msg_notify_t;

typedef struct yr_task_msg_t {
    void *source;
    void *msg;
    yr_uint8_t reason;
    yr_uint16_t notify;
} yr_task_msg_t;

typedef struct yr_task_t {
    yr_list_t list_node;

    void *sp;
    void *entry;
    void *param;
    void *stack_addr;
    yr_uint32_t stack_size;

    yr_uint8_t init_priority;
    yr_uint8_t current_priority;
    yr_uint32_t priority_mask; /* 永远等于 1 << current_priority */

    yr_uint32_t init_ticks;
    yr_uint32_t remaining_ticks;
    
    yr_uint32_t status;
    
    yr_uint8_t hold_mutex_count;
    yr_task_msg_t msg_info;
    yr_timer_t timer;
} yr_task_t;

typedef void (*yr_task_func_t)(void *param);

/**
 * @brief 初始化任务对象。
 * @param task 任务对象指针。
 * @param entry 任务入口函数。
 * @param param 传递给任务入口函数的参数。
 * @param stack_addr 任务栈起始地址。
 * @param stack_size 任务栈大小。
 * @param priority 初始优先级。
 * @param ticks 初始时间片长度。
 * @return 初始化结果。
 */
yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t ticks);

/**
 * @brief 初始化任务对象并使用默认时间片长度。
 * @param task 任务对象指针。
 * @param entry 任务入口函数。
 * @param param 传递给任务入口函数的参数。
 * @param stack_addr 任务栈起始地址。
 * @param stack_size 任务栈大小。
 * @param priority 初始优先级。
 * @return 初始化结果。
 */
yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority);

/**
 * @brief 删除任务。
 * @param task 任务对象指针。
 * @return 操作结果。
 */
yr_err_t yr_task_delete(yr_task_t *task);

/**
 * @brief 启动任务并交由调度器管理。
 * @param task 任务对象指针。
 * @return 操作结果。
 */
yr_err_t yr_task_start( yr_task_t *task);

/**
 * @brief 挂起任务。
 * @param task 任务对象指针。
 * @return 操作结果。
 */
yr_err_t yr_task_suspend( yr_task_t *task);

/**
 * @brief 让当前任务相对延时指定 tick 数。
 * @param ticks 延时 tick 数。
 */
void yr_task_sleep_ticks( yr_uint32_t ticks);

/**
 * @brief 让当前任务按周期方式延时。
 * @param pre_ticks 上一次唤醒时刻记录值。
 * @param inc_ticks 周期增量 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_task_sleep_until(yr_uint32_t *pre_ticks, yr_uint32_t inc_ticks);

/**
 * @brief 清理已终止但尚未彻底删除的任务。
 */
void yr_task_cleanup_defunct(void);

/**
 * @brief 获取或修改任务的当前控制信息。
 * @param task 任务对象指针。
 * @param cmd 控制命令。
 * @param arg 命令参数。
 * @param need_switch 若操作导致需要切换任务则置为 YR_TRUE。
 * @return 操作结果。
 */
yr_err_t yr_task_ctrl_current( yr_task_t *task, yr_uint32_t cmd, void *arg, yr_bool_t *need_switch);

/**
 * @brief 修改任务初始优先级。
 * @param task 任务对象指针。
 * @param priority 新优先级。
 * @return 操作结果。
 */
yr_err_t yr_task_set_priority( yr_task_t *task, yr_uint8_t priority);

/**
 * @brief 设置任务的阻塞消息信息。
 * @param task 任务对象指针。
 * @param source 消息来源对象。
 * @param msg 附加消息指针。
 * @param reason 阻塞原因。
 * @param notify 唤醒通知类型。
 * @return 操作结果。
 */
yr_err_t yr_task_set_msg( yr_task_t *task, void *source, void *msg, yr_uint8_t reason, yr_uint16_t notify);

#endif /* YUAN_RTOS_TASK_H */
