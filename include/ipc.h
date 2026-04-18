#ifndef YUAN_RTOS_IPC_H
#define YUAN_RTOS_IPC_H

#include "yr_config.h"

#if YR_SUPPORT_IPC

#include "list.h"
#include "portable.h"
#include "task.h"
#include "yr_def.h"

typedef enum yr_ipc_flag_t {
    YR_IPC_FLAG_NONE = 0,
    YR_IPC_FLAG_FIFO,
    YR_IPC_FLAG_PRIO,
} yr_ipc_flag_t;

typedef struct yr_ipc_t {
    yr_uint8_t is_valid;
    yr_uint32_t flag;
    yr_list_head_t blocked_list;
} yr_ipc_base_t;

/**
 * @brief 初始化 IPC 基础对象。
 * @param ipc_base IPC 基础对象指针。
 * @param flag 阻塞任务的排队策略。
 * @return 初始化结果。
 */
yr_err_t yr_ipc_init( yr_ipc_base_t *ipc_base, yr_uint32_t flag);

/**
 * @brief 将任务加入 IPC 阻塞队列。
 * @param ipc_base IPC 基础对象指针。
 * @param task 要阻塞的任务指针。
 * @return 操作结果。
 */
yr_err_t yr_ipc_block_task( yr_ipc_base_t *ipc_base, yr_task_t *task);

/**
 * @brief 恢复 IPC 阻塞队列中的所有任务。
 * @param ipc_base IPC 基础对象指针。
 * @return 操作结果。
 */
yr_err_t yr_ipc_resume_all( yr_ipc_base_t *ipc_base);

/**
 * @brief 重新整理阻塞任务在 IPC 队列中的顺序。
 * @param ipc_base IPC 基础对象指针。
 * @param task 需要重新排序的任务指针。
 * @return 操作结果。
 */
yr_err_t yr_ipc_reorder_blocked_task( yr_ipc_base_t *ipc_base, yr_task_t *task);

#endif /* YR_SUPPORT_IPC */

#endif /* YUAN_RTOS_IPC_H */
