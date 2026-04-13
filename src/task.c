#include "task.h"

yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size)
{
    if( task == NULL || entry == NULL || stack_addr == NULL )
        return YR_ERR_PARAM;

    task->stack_addr = stack_addr;
    task->stack_size = stack_size;

    task->sp = yr_task_stack_init( entry, (void*)0, param,(yr_uint8_t*)stack_addr + stack_size);

    return YR_ERR_OK;
}
