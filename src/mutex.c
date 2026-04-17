#include "mutex.h"
#include "scheduler.h"

yr_err_t yr_mutex_init( yr_mutex_t* mutex, yr_uint32_t flag)
{
    YR_PARAM_CHECK( mutex == NULL, YR_NULL);

    mutex->owner = NULL;
    mutex->hold = 0;
    mutex->original_priority = YR_MUTEX_PRIO_INVALID;

    return yr_ipc_init( &mutex->ipc_base, flag);
}

static void yr_mutex_restore_owner_priority( yr_mutex_t* mutex, yr_bool_t *need_switch)
{
    yr_uint8_t highest_blocked_priority;
    yr_list_t *blocked_list = mutex->ipc_base.blocked_list.next;

    YR_PARAM_CHECK( mutex == NULL ||
                    mutex->owner == NULL, YR_RETURN_NONE);
    YR_PARAM_CHECK( mutex->original_priority == YR_MUTEX_PRIO_INVALID, YR_RETURN_NONE);

    highest_blocked_priority = mutex->original_priority;

    while ( blocked_list != &mutex->ipc_base.blocked_list ) {
        yr_task_t *blocked_node = YR_LIST_ENTRY( blocked_list, yr_task_t, list_node);
        if ( blocked_node->current_priority < highest_blocked_priority ) 
            highest_blocked_priority = blocked_node->current_priority;
        blocked_list = blocked_list->next;
    }

    if( highest_blocked_priority != mutex->owner->current_priority ) 
        yr_task_ctrl( mutex->owner, YR_TASK_CTL_SET_PRIORITY, &highest_blocked_priority, need_switch);
}

yr_err_t yr_mutex_delete( yr_mutex_t* mutex)
{
    yr_uint32_t disirq;
    yr_bool_t need_switch = YR_FALSE;

    YR_PARAM_CHECK( mutex == NULL, YR_NULL);
    YR_PARAM_CHECK( mutex->ipc_base.is_valid == YR_FALSE, YR_OK);

    disirq = yr_irq_disable();

    if( !yr_list_isempty( &mutex->ipc_base.blocked_list) ) {
        need_switch = YR_TRUE;
        yr_ipc_resume_all( &mutex->ipc_base);
    }

    if( mutex->owner && mutex->original_priority != YR_MUTEX_PRIO_INVALID &&
        mutex->original_priority != mutex->owner->current_priority ) {
        yr_task_ctrl( mutex->owner, YR_TASK_CTL_SET_PRIORITY, &mutex->original_priority, &need_switch);
    }

    mutex->owner = NULL;
    mutex->hold = 0;
    mutex->original_priority = 0xFF;
    mutex->ipc_base.is_valid = YR_FALSE;
    mutex->ipc_base.flag = YR_IPC_FLAG_NONE;

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

yr_err_t yr_mutex_take( yr_mutex_t* mutex, yr_uint32_t wait_ticks)
{
    yr_uint32_t disirq = 0;
    yr_task_t *current_task = NULL;
    
    YR_PARAM_CHECK( mutex == NULL, YR_NULL);
    YR_PARAM_CHECK( mutex->ipc_base.is_valid == YR_FALSE, YR_NULL);

    disirq = yr_irq_disable();

    if( mutex->hold >=  YR_MUTEX_HOLD_MAX ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    current_task = yr_sched_get_current();
    YR_ASSERT( current_task != NULL );

    /* 如果当前任务持有锁，或者没有任务持有锁 */
    if( mutex->owner == current_task ) {
        mutex->hold++;
        yr_irq_enable(disirq);
        return YR_OK;
    } else if( mutex->owner == NULL && mutex->hold == 0 ) {
        mutex->hold = 1;
        mutex->owner = current_task;
        mutex->original_priority = current_task->current_priority;
        yr_irq_enable(disirq);
        return YR_OK;
    }

    if( wait_ticks == 0 ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    if( mutex->owner && current_task->current_priority < mutex->owner->current_priority ) {
        if( mutex->original_priority == YR_MUTEX_PRIO_INVALID )
            mutex->original_priority = mutex->owner->current_priority;
        yr_task_ctrl( mutex->owner, YR_TASK_CTL_SET_PRIORITY, &current_task->current_priority, NULL);
    }

    yr_task_set_block_info( current_task, (void*)&mutex->ipc_base, YR_TASK_BR_IPC, YR_TASK_BN_NONE);

    /* 加入信号量的阻塞队列 */
    yr_ipc_block_task( &mutex->ipc_base, current_task);

    /* 如果不希望最坏情况下“永远”等待，那么开启超时定时器 */
    if( wait_ticks != YR_WAIT_FOREVER ) {
        yr_timer_set_ticks( &current_task->timer, wait_ticks);
        yr_timer_start( &current_task->timer);
    }

    yr_irq_enable(disirq);

    /* 阻塞，切换到其它任务 */
    yr_sched_switch();

    disirq = yr_irq_disable();

    if( current_task->block_info.notify == YR_TASK_BN_WAIT_OK &&
        mutex->owner == current_task ) {
        yr_irq_enable(disirq);
        return YR_OK;
    } 

    yr_mutex_restore_owner_priority(mutex, NULL);
    yr_irq_enable(disirq);
    return YR_ERR;
}

yr_err_t yr_mutex_give( yr_mutex_t* mutex)
{
    yr_uint32_t disirq = 0;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *task = NULL, *current_task = NULL;

    YR_PARAM_CHECK( mutex == NULL, YR_NULL);
    YR_PARAM_CHECK( mutex->ipc_base.is_valid == YR_FALSE, YR_NULL);

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();
    YR_ASSERT( current_task != NULL );

    if( mutex->owner != current_task ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    if( mutex->hold > 0 )
        mutex->hold--;

    if( mutex->hold == 0 ) {
        if( mutex->original_priority != YR_MUTEX_PRIO_INVALID )
            yr_task_ctrl( mutex->owner, YR_TASK_CTL_SET_PRIORITY, &mutex->original_priority, &need_switch);
        mutex->owner = NULL;
        mutex->original_priority = YR_MUTEX_PRIO_INVALID;   
    } else {
        yr_irq_enable(disirq);
        return YR_OK;
    }

    if( !yr_list_isempty( &mutex->ipc_base.blocked_list ) ) {
        task = YR_LIST_ENTRY( mutex->ipc_base.blocked_list.next, yr_task_t, list_node);
        /* 脱离信号量阻塞队列，停止超时定时器，重新进入调度队列 */
        yr_list_delete_self( &task->list_node);
        task->status = YR_TASK_STATUS_READY;
        yr_task_set_block_info( task, NULL, YR_TASK_BR_NONE, YR_TASK_BN_WAIT_OK);
        yr_timer_stop(&task->timer);
        yr_sched_insert_task( task);

        /* 将锁的资源交给新任务 */
        mutex->owner = task;
        mutex->original_priority = task->current_priority;
        mutex->hold = 1;

        if( task->current_priority < current_task->current_priority )
            need_switch = YR_TRUE;
    }

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}
