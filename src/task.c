#include "task.h"
#include "yr_config.h"
#include "scheduler.h"

yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t tick)
{
    YR_PARAM_CHECK( task == NULL || entry == NULL || stack_addr == NULL, YR_NULL);
    YR_PARAM_CHECK( stack_size == 0 || priority >= YR_TASK_MAX_PRIORITY,YR_INVALID);

    task->stack_addr = stack_addr;
    task->stack_size = stack_size;

    task->sp = yr_task_stack_init( entry, (void*)0, param,(yr_uint8_t*)stack_addr + stack_size);

    task->init_priority = priority;
    task->current_priority = task->init_priority;
    task->priority_mask = (yr_uint32_t)1 << task->current_priority;

    task->init_tick = tick;
    task->remaining_tick = task->init_tick;
    task->status = YR_TASK_STATUS_READY;

    return YR_OK;
}

yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority)
{
    return yr_task_init( task, entry, param, stack_addr, stack_size, priority, YR_DEFAULT_TIME_SLICE_TICKS);
}

yr_err_t yr_task_start( yr_task_t *task)
{
    YR_PARAM_CHECK( task == NULL, YR_NULL);
    YR_PARAM_CHECK( task->status == YR_TASK_STATUS_DELETED, YR_INVALID);
    
    task->current_priority = task->init_priority;
    task->status = YR_TASK_STATUS_READY;
    task->remaining_tick = task->init_tick;

    yr_sched_insert_task(task);

    return YR_OK;
}
