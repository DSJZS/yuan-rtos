#include "idle.h"
#include "task.h"
#include "yr_config.h"

static yr_task_t yr_idle_task;
static yr_uint8_t yr_idle_stack[YR_IDLE_TASK_STACK_SZIE];

void yr_idle_task_entry(void *param)
{
    yr_task_cleanup_defunct();
    
    for(;;) {
        /* 空闲任务保持常驻，不允许返回 */
        /* 可在这里进入低功耗模式以降低功耗 */
    }
}

void yr_idle_task_init(void)
{
    yr_task_create( &yr_idle_task, yr_idle_task_entry, NULL, yr_idle_stack, sizeof(yr_idle_stack), YR_TASK_MAX_PRIORITY - 1);
    yr_task_start( &yr_idle_task);
}
