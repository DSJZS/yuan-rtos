#include "mutex.h"

#if YR_SUPPORT_MUTEX

#include "scheduler.h"

yr_err_t yr_mutex_init( yr_mutex_t* mutex, yr_uint32_t flag)
{
    YR_PARAM_CHECK( mutex == NULL, YR_NULL);

    mutex->owner = NULL;
    mutex->hold = 0;

    return yr_ipc_init( &mutex->ipc_base, flag);
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

    if( mutex->owner != NULL ) {
        if( mutex->owner->hold_mutex_count > 0 )
            mutex->owner->hold_mutex_count--;

        if( mutex->owner->hold_mutex_count == 0 &&
            mutex->owner->current_priority != mutex->owner->init_priority ) {
            yr_task_ctrl_current( mutex->owner, YR_TASK_CTL_SET_CUR_PRIORITY, &mutex->owner->init_priority, &need_switch);
        }
    }

    mutex->owner = NULL;
    mutex->hold = 0;
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
    YR_PARAM_CHECK( mutex->ipc_base.is_valid == YR_FALSE, YR_INVALID);

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
        current_task->hold_mutex_count++;
        yr_irq_enable(disirq);
        return YR_OK;
    }

    if( wait_ticks == 0 ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    if( mutex->owner && current_task->current_priority < mutex->owner->current_priority ) {
        yr_task_ctrl_current( mutex->owner, YR_TASK_CTL_SET_CUR_PRIORITY, &current_task->current_priority, NULL);
    }

    yr_task_set_msg( current_task, (void*)&mutex->ipc_base, NULL, YR_TASK_MR_IPC, YR_TASK_MN_NONE);

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

    if( current_task->msg_info.notify == YR_TASK_MN_WAIT_OK &&
        mutex->owner == current_task ) {
        yr_irq_enable(disirq);
        return YR_OK;
    } 

    yr_irq_enable(disirq);
    return YR_ERR;
}

yr_err_t yr_mutex_give( yr_mutex_t* mutex)
{
    yr_uint32_t disirq = 0;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *task = NULL, *current_task = NULL;

    YR_PARAM_CHECK( mutex == NULL, YR_NULL);
    YR_PARAM_CHECK( mutex->ipc_base.is_valid == YR_FALSE, YR_INVALID);

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
        if( current_task->hold_mutex_count > 0 )
            current_task->hold_mutex_count--;

        if( current_task->hold_mutex_count == 0 &&
            current_task->current_priority != current_task->init_priority ) {
            yr_task_ctrl_current( current_task, YR_TASK_CTL_SET_CUR_PRIORITY, &current_task->init_priority, &need_switch);
        }
        mutex->owner = NULL;
    } else {
        yr_irq_enable(disirq);
        return YR_OK;
    }

    if( !yr_list_isempty( &mutex->ipc_base.blocked_list ) ) {
        task = YR_LIST_ENTRY( mutex->ipc_base.blocked_list.next, yr_task_t, list_node);
        /* 脱离信号量阻塞队列，停止超时定时器，重新进入调度队列 */
        yr_list_delete_self( &task->list_node);
        task->status = YR_TASK_STATUS_READY;
        yr_task_set_msg( task, NULL, NULL, YR_TASK_MR_NONE, YR_TASK_MN_WAIT_OK);
        yr_timer_stop(&task->timer);
        yr_sched_insert_task( task);

        /* 将锁的资源交给新任务 */
        mutex->owner = task;
        mutex->hold = 1;
        task->hold_mutex_count++;

        if( task->current_priority < current_task->current_priority )
            need_switch = YR_TRUE;
    }

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

#endif /* YR_SUPPORT_MUTEX */
