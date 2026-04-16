#include "timer.h"
#include "portable.h"
#include "task.h"
#include "scheduler.h"

volatile yr_uint32_t yr_current_ticks = 0;
yr_list_head_t yr_timer_list;

/* 这个函数不是原子性的 */
yr_uint32_t yr_get_current_ticks(void)
{
    return yr_current_ticks;
}

void yr_timer_list_init(void) 
{
    yr_list_init( &yr_timer_list);
}

yr_err_t yr_timer_init( yr_timer_t *timer, yr_timer_func_t func, void *param, yr_uint32_t ticks)
{
    YR_PARAM_CHECK( timer == NULL || func == NULL, YR_NULL);
    
    yr_list_init( &timer->list_node);

    timer->timeout_func = func;
    timer->param = param;
    timer->init_ticks = ticks;
    timer->timeout_ticks = 0;

    return YR_OK;
}

void yr_timer_remove( yr_timer_t *timer)
{
    yr_uint32_t disirq = 0;

    disirq = yr_irq_disable();

    yr_list_delete_self( &timer->list_node);

    yr_irq_enable(disirq);
}

yr_err_t yr_timer_start( yr_timer_t *timer)
{
    yr_uint32_t disirq = 0;
    yr_list_t *timer_node;
    
    YR_PARAM_CHECK( timer == NULL, YR_NULL);

    disirq = yr_irq_disable();

    /* 避免重复的节点 */
    yr_timer_remove( timer);

    timer->timeout_ticks = yr_current_ticks + timer->init_ticks;
    
    for( timer_node = yr_timer_list.next; timer_node != &yr_timer_list; timer_node = timer_node->next ) {
        yr_timer_t *ptimer = YR_LIST_ENTRY( timer_node, yr_timer_t, list_node);
        if( (yr_int32_t)( ptimer->timeout_ticks - timer->timeout_ticks ) > 0 )
            break;
    }
    yr_list_insert_before( timer_node, &timer->list_node);

    yr_irq_enable(disirq);
    return YR_OK;
}

yr_err_t yr_timer_stop( yr_timer_t *timer)
{
    YR_PARAM_CHECK( timer == NULL, YR_NULL);

    yr_timer_remove( timer);

    return YR_OK;
}

yr_err_t yr_timer_set_ticks( yr_timer_t *timer, yr_uint32_t ticks)
{
    YR_PARAM_CHECK( timer == NULL, YR_NULL);
    timer->init_ticks = ticks;
    return YR_OK;
}

void yr_timer_check(void)
{
    yr_uint32_t disirq = 0;
    yr_list_t *timer_node;
    yr_list_head_t timeout_list;
    yr_timer_t *ptimer;

    yr_list_init( &timeout_list);

    disirq = yr_irq_disable();

    while(!yr_list_isempty(&yr_timer_list)) {
        timer_node = yr_timer_list.next;
        ptimer = YR_LIST_ENTRY( timer_node, yr_timer_t, list_node);

        if( (yr_int32_t)( yr_current_ticks -  ptimer->timeout_ticks ) >= 0 ) {
            /* 超时后脱离 yr_timer_list 的管理*/
            yr_list_delete_self( timer_node);
            yr_list_insert_before( &timeout_list, timer_node);
        } else {
            /* yr_timer_list 是一个小数在前的有序链表，故这里可以直接退出循环 */
            break;
        }
    }

    yr_irq_enable(disirq);

    while (!yr_list_isempty(&timeout_list)) {
        timer_node = timeout_list.next;
        ptimer = YR_LIST_ENTRY(timer_node, yr_timer_t, list_node);

        /* 防止 param 涉及 timer_node 时会产生的一系列错误  
         * 比如有时 timeout_func 中会利用 yr_timer_start
         * 以实现重复周期软件定时器
         */
        yr_list_delete_self(timer_node);

        if (ptimer->timeout_func != NULL)
            ptimer->timeout_func(ptimer->param);
    }
}

void yr_tick_update(void)
{
    yr_uint32_t disirq = 0;
    yr_task_t * current_task;

    disirq = yr_irq_disable();

    ++yr_current_ticks;

    current_task = yr_sched_get_current();
    if( current_task == NULL ) {
        yr_irq_enable(disirq);
        return;
    }

    if (current_task->status != YR_TASK_STATUS_RUNNING) {
        yr_irq_enable(disirq);
        yr_timer_check();
        return;
    }

    --current_task->remaining_ticks;
    if( current_task->remaining_ticks == 0 ) {
        current_task->remaining_ticks = current_task->init_ticks;
        yr_irq_enable(disirq);
        yr_sched_yield();
    } else {
        yr_irq_enable(disirq);
    }

    yr_timer_check();
}

/* 执行这个函数时， param 必须指向一个task */
void yr_timeout_default_func(void *param)
{
    yr_task_t *task = (yr_task_t *)param;
    yr_uint32_t disirq;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *current_task;

    YR_ASSERT(task != NULL);

    disirq = yr_irq_disable();

    /* 如果不处于阻塞状态，超时回调无权更改状态 */
    if (task->status != YR_TASK_STATUS_BLOCKED) {
        yr_irq_enable(disirq);
        return;
    }

    current_task = yr_sched_get_current();
    
    /* 确保脱离其它模块的控制，比如 IPC 的 blocked_list  */
    yr_list_delete_self(&task->list_node);
    task->status = YR_TASK_STATUS_READY;
    task->sync_notify = YR_TASK_SYNC_NOTIFY_WAIT_TIMEOUT;
    yr_sched_insert_task(task);

    if (current_task != NULL &&
        task->current_priority < current_task->current_priority)
        need_switch = YR_TRUE;

    yr_irq_enable(disirq);

    if (need_switch)
        yr_sched_switch();
}

