#include "task.h"
#include "yr_config.h"
#include "scheduler.h"
#include "yr_def.h"

yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t ticks)
{
    YR_PARAM_CHECK( task == NULL || entry == NULL || stack_addr == NULL, YR_NULL);
    YR_PARAM_CHECK( stack_size == 0 || priority >= YR_TASK_MAX_PRIORITY,YR_INVALID);

    task->stack_addr = stack_addr;
    task->stack_size = stack_size;

    task->sp = yr_task_stack_init( entry, (void*)0, param,(yr_uint8_t*)stack_addr + stack_size);

    task->init_priority = priority;
    task->current_priority = task->init_priority;
    task->priority_mask = (yr_uint32_t)1 << task->current_priority;

    task->init_ticks = ticks;
    task->remaining_ticks = task->init_ticks;

    if ( yr_timer_init(&(task->timer), yr_timeout_default_func, task, ticks) != YR_OK)
        return YR_INVALID;

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
    task->remaining_ticks = task->init_ticks;

    yr_sched_insert_task(task);

    return YR_OK;
}

void yr_task_sleep_ticks( yr_uint32_t ticks) 
{
    yr_uint32_t disirq = 0;
    yr_task_t *current_task;

    /* ticks == 0 的情况不适用 YR_PARAM_CHECK，应等价与 yr_sched_switch */
    if( ticks == 0 ) {
        yr_sched_switch();
        return;
    }

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();

    // yr_list_delete_self( &current_task->list_node);
    /* 这里不能使用 yr_list_delete_self，需要用到调度器删除
     * 确保 yr_thread_ready_priority_group 位图的正确性 
     */
    yr_sched_delete_task( current_task);
    current_task->status = YR_TASK_STATUS_BLOCKED;

    yr_timer_stop( &current_task->timer);
    yr_timer_set_ticks( &current_task->timer, ticks);
    yr_timer_start( &current_task->timer);

    yr_irq_enable(disirq);
    yr_sched_switch();
}

yr_err_t yr_task_sleep_until(yr_uint32_t *pre_ticks, yr_uint32_t inc_ticks)
{
    yr_uint32_t disirq;
    yr_uint32_t current_ticks;
    yr_uint32_t sleep_ticks;

    YR_PARAM_CHECK(pre_ticks == NULL, YR_NULL);

    *pre_ticks += inc_ticks;

    disirq = yr_irq_disable();

    current_ticks = yr_get_current_ticks();
    if ((yr_int32_t)(*pre_ticks - current_ticks) <= 0)
    {
        yr_irq_enable(disirq);
        return YR_OK;
    }

    sleep_ticks = *pre_ticks - current_ticks;

    yr_irq_enable(disirq);

    yr_task_sleep_ticks(sleep_ticks);

    return YR_OK;
}
