#include "ipc.h"

#if YR_SUPPORT_IPC

#include "scheduler.h"

static void yr_ipc_insert_blocked_task( yr_ipc_base_t *ipc_base, yr_task_t *task)
{
    yr_list_t *plist = NULL;

    switch ( ipc_base->flag ) {
        case YR_IPC_FLAG_PRIO:
            plist = ipc_base->blocked_list.next;

            while( plist != &ipc_base->blocked_list ) {
                yr_task_t *ptask = YR_LIST_ENTRY( plist, yr_task_t, list_node);

                /* 注意这个项目是 0 为最大优先级，故这里 priority 变量小的，优先级大，排前面*/
                if( task->current_priority < ptask->current_priority ) {
                    yr_list_insert_before( &ptask->list_node, &task->list_node);
                    break;
                }
                plist = plist->next;
            }

            if( plist == &ipc_base->blocked_list )
                yr_list_insert_before( &ipc_base->blocked_list, &task->list_node);

            break;

        /* 默认先进先出 */
        case YR_IPC_FLAG_FIFO:
        case YR_IPC_FLAG_NONE:
        default:
            yr_list_insert_before( &ipc_base->blocked_list, &task->list_node);
            break;
    }
}

yr_err_t yr_ipc_init( yr_ipc_base_t *ipc_base, yr_uint32_t flag)
{
    YR_PARAM_CHECK( ipc_base == NULL, YR_NULL );
    YR_PARAM_CHECK( flag != YR_IPC_FLAG_FIFO &&
                    flag != YR_IPC_FLAG_PRIO , YR_INVALID );

    ipc_base->is_valid = YR_TRUE;
    ipc_base->flag = flag;
    yr_list_init( &ipc_base->blocked_list);

    return YR_OK;
}

yr_err_t yr_ipc_block_task( yr_ipc_base_t *ipc_base, yr_task_t *task)
{
    yr_uint32_t disirq = 0;

    YR_PARAM_CHECK( ipc_base == NULL, YR_NULL );
    YR_PARAM_CHECK( task == NULL, YR_NULL );

    disirq = yr_irq_disable();

    yr_sched_remove_task( task);
    task->status = YR_TASK_STATUS_BLOCKED;
    yr_ipc_insert_blocked_task( ipc_base, task );

    yr_irq_enable(disirq);

    return YR_OK;
}

yr_err_t yr_ipc_resume_all( yr_ipc_base_t *ipc_base)
{
    yr_uint32_t disirq;
    yr_task_t *task;
    yr_list_head_t*  list_head;

    YR_PARAM_CHECK( ipc_base == NULL, YR_NULL );

    list_head = &ipc_base->blocked_list;

    while (!yr_list_isempty( list_head ))
    {
        disirq = yr_irq_disable();

        task = YR_LIST_ENTRY( list_head->next, yr_task_t, list_node);
        yr_list_delete_self(&task->list_node);
        yr_timer_stop(&task->timer);
        task->status = YR_TASK_STATUS_READY;
        yr_task_set_msg( task, NULL, NULL, YR_TASK_MR_NONE, YR_TASK_MN_WAIT_IPC_DELETED);
        yr_sched_insert_task( task);

        yr_irq_enable(disirq);
    }

    return YR_OK;
}

yr_err_t yr_ipc_reorder_blocked_task( yr_ipc_base_t *ipc_base, yr_task_t *task)
{
    yr_uint32_t disirq;

    YR_PARAM_CHECK( ipc_base == NULL, YR_NULL );
    YR_PARAM_CHECK( task == NULL, YR_NULL );

    if( ipc_base->flag != YR_IPC_FLAG_PRIO )
        return YR_OK;

    disirq = yr_irq_disable();

    yr_list_delete_self( &task->list_node );
    yr_ipc_insert_blocked_task( ipc_base, task );

    yr_irq_enable(disirq);

    return YR_OK;
}

#endif /* YR_SUPPORT_IPC */
