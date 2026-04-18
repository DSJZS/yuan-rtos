#ifndef YUAN_RTOS_SCHEDULER_H
#define YUAN_RTOS_SCHEDULER_H

#include "yr_def.h"
#include "task.h"

/**
 * @brief 获取当前运行任务。
 * @return 当前任务指针。
 */
yr_task_t* yr_sched_get_current(void);

/**
 * @brief 初始化调度器。
 */
void yr_sched_init(void);

/**
 * @brief 启动调度器并切换到第一个任务。
 */
void yr_sched_start(void);

/**
 * @brief 执行一次任务切换。
 */
void yr_sched_switch(void);

/**
 * @brief 将任务插入就绪队列。
 * @param task 待插入任务指针。
 * @return 操作结果。
 */
yr_err_t yr_sched_insert_task( yr_task_t* task);

/**
 * @brief 将任务移出就绪队列。
 * @param task 待移除任务指针。
 * @return 操作结果。
 */
yr_err_t yr_sched_remove_task( yr_task_t* task);

/**
 * @brief 当前任务主动让出处理器。
 */
void yr_sched_yield(void);

#endif /* YUAN_RTOS_SCHEDULER_H */
