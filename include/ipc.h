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

yr_err_t yr_ipc_init( yr_ipc_base_t *ipc_base, yr_uint32_t flag);
yr_err_t yr_ipc_block_task( yr_ipc_base_t *ipc_base, yr_task_t *task);
yr_err_t yr_ipc_resume_all( yr_ipc_base_t *ipc_base);
yr_err_t yr_ipc_reorder_blocked_task( yr_ipc_base_t *ipc_base, yr_task_t *task);

#endif /* YR_SUPPORT_IPC */

#endif /* YUAN_RTOS_IPC_H */
