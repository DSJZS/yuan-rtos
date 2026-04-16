#include "semaphore.h"

#if YR_SUPPORT_SEMAPHORE

#include "scheduler.h"

yr_err_t yr_semaphore_init( yr_semaphore_t* sem, yr_uint16_t max_count,yr_uint16_t init_count, yr_uint32_t flag)
{
    YR_PARAM_CHECK( sem == NULL, YR_NULL);
    YR_PARAM_CHECK( max_count == 0 ||
                    init_count > max_count, YR_INVALID);

    sem->max_count = max_count;
    sem->current_count = init_count;

    return yr_ipc_init( &sem->ipc_base, flag);
}

/* 删除一个信号量，解除阻塞队列里的任务让他们变为 READY 状态 */
yr_err_t yr_semaphore_delete( yr_semaphore_t* sem)
{
    yr_uint32_t disirq;
    yr_bool_t need_switch = YR_FALSE;
    
    YR_PARAM_CHECK( sem == NULL, YR_NULL);

    disirq = yr_irq_disable();

    if( !yr_list_isempty( &sem->ipc_base.blocked_list) ) {
        need_switch = YR_TRUE;
        yr_ipc_resume_all( &sem->ipc_base);
    }

    sem->max_count = 0;
    sem->current_count = 0;
    sem->ipc_base.is_valid = YR_FALSE;
    sem->ipc_base.flag = YR_IPC_FLAG_NONE;

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

/* 获取者尝试获取信号量，如果没有则最多阻塞指定的时间 ticks */
yr_err_t yr_semaphore_take( yr_semaphore_t* sem, yr_uint32_t wait_ticks)
{
    yr_uint32_t disirq = 0;
    yr_task_t *current_task = NULL;

    YR_PARAM_CHECK( sem == NULL, YR_NULL);
    YR_PARAM_CHECK( sem->ipc_base.is_valid == YR_FALSE ||
                    sem->max_count == 0, YR_INVALID);

    disirq = yr_irq_disable();

    /* 有信号量，取走并返回 */
    if( sem->current_count > 0 ) {
        sem->current_count--;
        yr_irq_enable(disirq);
        return YR_OK;
    }

    /* 没信号量且不想等待 */
    if( wait_ticks == 0 ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    current_task = yr_sched_get_current();
    YR_ASSERT( current_task != NULL );

    current_task->sync_notify = YR_TASK_SYNC_NOTIFY_NONE;

    /* 加入信号量的阻塞队列 */
    yr_ipc_block_task( &sem->ipc_base, current_task);

    /* 如果不希望最坏情况下“永远”等待，那么开启超时定时器 */
    if( wait_ticks != YR_WAIT_FOREVER ) {
        yr_timer_set_ticks( &current_task->timer, wait_ticks);
        yr_timer_start( &current_task->timer);
    }

    yr_irq_enable(disirq);

    /* 阻塞，切换到其它任务 */
    yr_sched_switch();

    /* 阻塞结束返回到这里 */
    disirq = yr_irq_disable();

    if( current_task->sync_notify == YR_TASK_SYNC_NOTIFY_WAIT_OK ) {
        yr_irq_enable(disirq);
        return YR_OK;
    }

    yr_irq_enable(disirq);
    return YR_ERR;
}

/* 释放信号量，如果达到最大值直接返回错误(因为理论上在合理的使用环境下信号量不应该超过上限) */
yr_err_t yr_semaphore_give( yr_semaphore_t* sem)
{
    yr_uint32_t disirq;
    yr_bool_t need_switch = YR_FALSE;
    yr_task_t *task = NULL, *current_task = NULL;

    YR_PARAM_CHECK( sem == NULL, YR_NULL);
    YR_PARAM_CHECK( sem->ipc_base.is_valid == YR_FALSE ||
                    sem->max_count == 0, YR_INVALID);
    
    disirq = yr_irq_disable();

    if( sem->current_count >= sem->max_count ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    /* 如果有任务等待信号量，直接将资源交与策略上优先级最高的任务 */
    if( !yr_list_isempty( &sem->ipc_base.blocked_list ) ) {
        current_task = yr_sched_get_current();

        YR_ASSERT( current_task != NULL );

        task = YR_LIST_ENTRY( sem->ipc_base.blocked_list.next, yr_task_t, list_node);
        /* 脱离信号量阻塞队列，停止超时定时器，重新进入调度队列 */
        yr_list_delete_self( &task->list_node);
        task->status = YR_TASK_STATUS_READY;
        task->sync_notify = YR_TASK_SYNC_NOTIFY_WAIT_OK;
        yr_timer_stop(&task->timer);
        yr_sched_insert_task( task);

        if( task->current_priority < current_task->current_priority )
            need_switch = YR_TRUE;
    } else {
        sem->current_count++;
    }

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

#endif /* YR_SUPPORT_SEMAPHORE */
