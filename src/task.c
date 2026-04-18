#include "task.h"
#include "yr_config.h"
#include "scheduler.h"
#include "yr_def.h"
#include "ipc.h"

static void __task_exit(void);
static void __task_delete(yr_task_t *task);

extern yr_list_head_t yr_task_defunct_list;

/* 用于初始化任务 ( 只有还未被初始化，或者 DELETED 状态的任务可以用这个函数 )
 * 
 */
yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t ticks)
{
    YR_PARAM_CHECK( task == NULL || entry == NULL || stack_addr == NULL, YR_NULL);
    YR_PARAM_CHECK( stack_size == 0 || priority >= YR_TASK_MAX_PRIORITY,YR_INVALID);

    task->stack_addr = stack_addr;
    task->stack_size = stack_size;
    yr_list_init( &task->list_node );

    task->sp = yr_task_stack_init( entry, __task_exit, param,(yr_uint8_t*)stack_addr + stack_size);

    task->init_priority = priority;
    task->current_priority = task->init_priority;
    task->hold_mutex_count = 0;
    task->priority_mask = (yr_uint32_t)1 << task->current_priority;

    task->init_ticks = ticks;
    task->remaining_ticks = task->init_ticks;

    if ( yr_timer_init(&(task->timer), yr_timeout_default_func, task, ticks) != YR_OK)
        return YR_ERR;

    task->status = YR_TASK_STATUS_INIT;

    yr_task_set_block_info( task, NULL, YR_TASK_BR_NONE, YR_TASK_BN_NONE);

    return YR_OK;
}

/* 用于初始化任务 ( 只有还未被初始化，或者 DELETED 状态的任务可以用这个函数 )
 * 与 yr_task_init 区别在于本函数使用默认时间片长度
 * 建议项目中使用本函数而非 yr_task_init，以保证以优先级为主的调度原则
 */
yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority)
{
    return yr_task_init( task, entry, param, stack_addr, stack_size, priority, YR_DEFAULT_TIME_SLICE_TICKS);
}

/* 用于将一个任务开始被调度( 只有 INIT，或者 SUSPENDED 状态的任务可以用这个函数 )
 * 这个函数是原子的
 */
yr_err_t yr_task_start(yr_task_t *task)
{
    yr_uint32_t disirq;
    yr_task_t *current_task;
    yr_bool_t need_switch = YR_FALSE;

    YR_PARAM_CHECK( task == NULL, YR_NULL );
    YR_PARAM_CHECK( task->status != YR_TASK_STATUS_INIT &&
                    task->status != YR_TASK_STATUS_SUSPENDED, YR_INVALID );

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();

    if (task == current_task)
    {
        yr_irq_enable(disirq);
        return YR_OK;
    }

    if (task->status == YR_TASK_STATUS_INIT)
    {
        task->current_priority = task->init_priority;
        task->remaining_ticks = task->init_ticks;
    }

    task->status = YR_TASK_STATUS_READY;
    yr_sched_insert_task(task);

    if (current_task != NULL &&
        task->current_priority < current_task->current_priority)
        need_switch = YR_TRUE;

    yr_irq_enable(disirq);

    if (need_switch)
        yr_sched_switch();

    return YR_OK;
}

/* 用于让执行该任务的函数进行相对延时，暂时进入 BLOCKED 状态
 * 非任务上下文，比如中断中不可调用这个函数
 * 这个函数是原子的
 */
void yr_task_sleep_ticks( yr_uint32_t ticks) 
{
    yr_uint32_t disirq = 0;
    yr_task_t *current_task;

    /* ticks == 0 的情况不适用 YR_PARAM_CHECK，应等价与 yr_sched_switch */
    if( ticks == 0 ) {
        yr_sched_switch();
        return;
    }

    current_task = yr_sched_get_current();

    disirq = yr_irq_disable();

    /* 这里不能使用 yr_list_delete_self，需要用到调度器删除
     * 确保 yr_thread_ready_priority_group 位图的正确性 
     */
    yr_sched_remove_task( current_task);
    current_task->status = YR_TASK_STATUS_BLOCKED;
    yr_task_set_block_info( current_task, (void*)&current_task->timer, YR_TASK_BR_SLEEP, YR_TASK_BN_NONE);

    yr_timer_stop( &current_task->timer);
    yr_timer_set_ticks( &current_task->timer, ticks);
    yr_timer_start( &current_task->timer);

    yr_irq_enable(disirq);
    yr_sched_switch();
}

/* 用于让执行该任务的函数进行周期延时，暂时进入 BLOCKED 状态
 * 非任务上下文，比如中断中不可调用这个函数
 * 这个函数是原子的
 */
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
    } else {
        yr_irq_enable(disirq);
    }

    sleep_ticks = *pre_ticks - current_ticks;

    yr_task_sleep_ticks(sleep_ticks);

    return YR_OK;
}

/* 用于删除指定的(用户创建的)任务。
 * 非用户创建任务不可删除( 比如 idle 任务 )
 * 当 task == NULL 时，表示删除当前任务 
 */
yr_err_t yr_task_delete(yr_task_t *task)
{
    yr_uint32_t disirq = 0;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *current_task = NULL;

    YR_PARAM_CHECK( task == NULL, YR_NULL );

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();

    if ( task == NULL )
        task = current_task;

    YR_ASSERT( task != NULL );

    if (task->status == YR_TASK_STATUS_TERMINATED ||
        task->status == YR_TASK_STATUS_DELETED ) {
        yr_irq_enable(disirq);
        return YR_OK;
    }
        
    if (task == current_task )
        need_switch = YR_TRUE;

    __task_delete(task);

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}



/* 这个函数用于在 idle 任务中，将所有 TERMINATED 状态的任务转为 DELETED 状态
 */
void yr_task_cleanup_defunct(void)
{
    yr_uint32_t disirq;
    disirq = yr_irq_disable();

    while (!yr_list_isempty(&yr_task_defunct_list))
    {
        yr_task_t *task = YR_LIST_ENTRY(yr_task_defunct_list.next, yr_task_t, list_node);
        task->status = YR_TASK_STATUS_DELETED;
        yr_list_delete_self(&(task->list_node));
    }

    yr_irq_enable(disirq);
}

/* 用于暂停指定的(用户创建的)任务。
 * 非用户创建任务不可暂停( 比如 idle 任务 )
 * 当 task == NULL 时，表示暂停当前任务 
 */
yr_err_t yr_task_suspend( yr_task_t *task)
{
    yr_uint32_t disirq = 0;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *current_task = NULL;

    YR_PARAM_CHECK( task == NULL, YR_NULL );

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();

    if ( task == NULL )
        task = current_task;

    YR_ASSERT( task != NULL );

    if ( task->status == YR_TASK_STATUS_SUSPENDED ) {
        yr_irq_enable(disirq);
        return YR_OK;
    } else if ( task->status == YR_TASK_STATUS_READY ||
                task->status == YR_TASK_STATUS_RUNNING ) {
        if (task == current_task)
            need_switch = YR_TRUE;

        yr_sched_remove_task(task);
        task->status = YR_TASK_STATUS_SUSPENDED;
    } else if (task->status == YR_TASK_STATUS_BLOCKED) {
        yr_timer_stop(&task->timer);
        yr_list_delete_self(&task->list_node);
        task->status = YR_TASK_STATUS_SUSPENDED;
    } else {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

yr_err_t yr_task_ctrl_current( yr_task_t *task, yr_uint32_t cmd, void *arg, yr_bool_t *need_switch)
{
    yr_uint32_t disirq = 0;
    yr_task_t *current_task = NULL;
    yr_uint8_t old_priority = 0;
    yr_uint8_t priority = 0;

    YR_PARAM_CHECK( task == NULL, YR_NULL);
    YR_PARAM_CHECK( cmd >= YR_TASK_CTL_CUR_CHECK, YR_INVALID);

    switch (cmd) {
        case YR_TASK_CTL_GET_CUR_STATUS:
            if (arg) *(yr_uint32_t *)arg = task->status;
            return YR_OK;
        case YR_TASK_CTL_GET_CUR_PRIORITY:
            if (arg) *(yr_uint32_t *)arg = task->current_priority;
            return YR_OK;
        case YR_TASK_CTL_SET_CUR_PRIORITY:
            YR_PARAM_CHECK( arg == NULL, YR_NULL );

            priority = *(yr_uint8_t *)arg;
            YR_PARAM_CHECK( priority >= YR_TASK_MAX_PRIORITY, YR_INVALID );

            if( task->current_priority == priority )
                return YR_OK;

            disirq = yr_irq_disable();
            current_task = yr_sched_get_current();
            old_priority = task->current_priority;

            task->current_priority = priority;
            task->priority_mask = (yr_uint32_t)1 << task->current_priority;

            switch( task->status ) {
                case YR_TASK_STATUS_READY:
                case YR_TASK_STATUS_RUNNING:
                    yr_sched_remove_task( task );
                    yr_sched_insert_task( task );
                    break;

                case YR_TASK_STATUS_BLOCKED:
                    if( task->block_info.reason == YR_TASK_BR_IPC &&
                        task->block_info.source != NULL )
                        yr_ipc_reorder_blocked_task( (yr_ipc_base_t *)task->block_info.source, task );
                    break;

                default:
                    break;
            }

            if( need_switch != NULL ) {
                if( current_task != NULL ) {
                    if( task == current_task &&
                        task->status == YR_TASK_STATUS_RUNNING &&
                        old_priority < priority ) {
                        *need_switch = YR_TRUE;
                    } else if( task->status == YR_TASK_STATUS_READY &&
                               task->current_priority < current_task->current_priority ) {
                        *need_switch = YR_TRUE;
                    }
                }
            }

            yr_irq_enable(disirq);
            return YR_OK;
        default:
            return YR_INVALID;
    }
}

yr_err_t yr_task_set_priority( yr_task_t *task, yr_uint8_t priority)
{
    yr_bool_t need_switch = YR_FALSE;
    yr_err_t result = YR_OK;

    YR_PARAM_CHECK( task == NULL, YR_NULL );
    YR_PARAM_CHECK( priority >= YR_TASK_MAX_PRIORITY, YR_INVALID );

    yr_uint32_t disirq;

    disirq = yr_irq_disable();

    task->init_priority = priority;

    if( task->hold_mutex_count == 0 ||
        task->init_priority < task->current_priority ) {
        result = yr_task_ctrl_current( task, YR_TASK_CTL_SET_CUR_PRIORITY, &(task->init_priority), &need_switch );
        if( result != YR_OK ) {
            yr_irq_enable(disirq);
            return result;
        }            
    }

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

yr_err_t yr_task_set_block_info( yr_task_t *task, void *source, yr_uint8_t reason, yr_uint16_t notify)
{
    YR_PARAM_CHECK( task == NULL, YR_NULL);
    YR_PARAM_CHECK( reason >= YR_TASK_BR_CHECK ||
                    notify >= YR_TASK_BN_CHECK, YR_INVALID);

    task->block_info.source = source;
    task->block_info.reason = reason;
    task->block_info.notify = notify;

    return YR_OK;
}

/* 任务默认出口函数
 * 当前任务 return 时会执行该函数删除自己
 * 只能通过 return 来执行，不可以暴露外部接口
 * 这个函数是原子的
 */
static void __task_exit(void)
{
    yr_task_t *current_task;
    yr_uint32_t disirq;

    current_task = yr_sched_get_current();
    YR_ASSERT(current_task != NULL);

    disirq = yr_irq_disable();

    __task_delete(current_task);

    yr_irq_enable(disirq);

    yr_sched_switch();

    for (;;)
    {
        /* do nothing */;
    }
}

/* 用于让一个任务不再被调度并转为 TERMINATED 状态等待删除
 * 这个函数不是原子的，不可以暴露外部接口
 *
 * 之所以不直接转为 DELETED 状态一部到位，是因为可能存在一个函数自己删除自己的情况，
 * 项目纯使用静态空间还好，如果未来实现利用动态分配的空间，会出现大问题，
 * 当他试图删除动态分配的栈、上下文结构体时，当前函数相关的数据、代码、返回路径可能仍在这些资源里，
 * 释放后可能会涉及非法访问的问题，严重时会直接进入硬件错误中断。
 *
 * 归根到底，由于后续扩展可能导致执行该函数时的上下文过于复杂，难以判断是否可以进行 delete 操作，
 * 因此规定这里只是标记，实际的删除操作统一在 yr_task_cleanup_defunct 中进行，方便后续扩展。
 */
static void __task_delete(yr_task_t *task)
{
    switch (task->status) {
        case YR_TASK_STATUS_READY:
        case YR_TASK_STATUS_RUNNING:
            yr_sched_remove_task(task);
            break;

        case YR_TASK_STATUS_BLOCKED:
        case YR_TASK_STATUS_SUSPENDED:
            yr_list_delete_self(&task->list_node);
            break;

        case YR_TASK_STATUS_INIT:
            break;
            
        case YR_TASK_STATUS_TERMINATED:
        case YR_TASK_STATUS_DELETED:
        default:
            return;
    }

    yr_timer_stop(&task->timer);
    task->status = YR_TASK_STATUS_TERMINATED;
    yr_list_insert_before(&yr_task_defunct_list, &task->list_node);
}