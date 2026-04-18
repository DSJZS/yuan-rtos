#include "queue.h"
#include "scheduler.h"
#include <string.h>

static void __queue_reset( yr_queue_t *queue);
static void __queue_write_item( yr_queue_t *queue, const void *item );
static void __queue_read_item( yr_queue_t *queue, void *item );
static yr_err_t __queue_resume_one( yr_ipc_base_t *ipc_base, yr_task_t *current_task, yr_bool_t *need_switch );

/* buffer_size 要为 item_size 的整数倍 */
yr_err_t yr_queue_init( yr_queue_t *queue, yr_uint32_t item_size, yr_uint8_t *buffer, yr_uint32_t buffer_size, yr_uint32_t flag)
{
    yr_err_t result = YR_OK;
    yr_uint32_t capacity = 0;

    YR_PARAM_CHECK( queue == NULL ||
                    buffer == NULL, YR_NULL);
    YR_PARAM_CHECK( item_size == 0 ||
                    buffer_size == 0 ||
                    (buffer_size % item_size) != 0, YR_INVALID );

    capacity = buffer_size / item_size;
    YR_PARAM_CHECK( capacity == 0, YR_INVALID );

    queue->buffer = buffer;
    queue->capacity = capacity;
    queue->item_size = item_size;

    __queue_reset(queue);

    result =  yr_ipc_init( &queue->send_ipc, flag);
    if( result != YR_OK )
        return result;
    result =  yr_ipc_init( &queue->receive_ipc, flag);
    if( result != YR_OK )
        return result;

    return YR_OK;
}

yr_err_t yr_queue_delete( yr_queue_t *queue)
{
    yr_uint32_t disirq;
    yr_bool_t need_switch = YR_FALSE;

    YR_PARAM_CHECK( queue == NULL, YR_NULL);
    YR_PARAM_CHECK( queue->send_ipc.is_valid == YR_FALSE ||
                    queue->receive_ipc.is_valid == YR_FALSE, YR_OK);

    disirq = yr_irq_disable();

    if( !yr_list_isempty( &queue->send_ipc.blocked_list) ) {
        need_switch = YR_TRUE;
        yr_ipc_resume_all( &queue->send_ipc);
    }

    if( !yr_list_isempty( &queue->receive_ipc.blocked_list) ) {
        need_switch = YR_TRUE;
        yr_ipc_resume_all( &queue->receive_ipc);
    }

    __queue_reset(queue);
    queue->buffer = NULL;
    queue->capacity = 0;
    queue->item_size = 0;

    queue->send_ipc.is_valid = YR_FALSE;
    queue->send_ipc.flag = YR_IPC_FLAG_NONE;

    queue->receive_ipc.is_valid = YR_FALSE;
    queue->receive_ipc.flag = YR_IPC_FLAG_NONE;

    yr_irq_enable(disirq);

    if( need_switch )
        yr_sched_switch();

    return YR_OK;
}

yr_err_t yr_queue_reset( yr_queue_t *queue)
{
    yr_uint32_t disirq;

    YR_PARAM_CHECK( queue == NULL, YR_NULL);
    YR_PARAM_CHECK( queue->send_ipc.is_valid == YR_FALSE ||
                    queue->receive_ipc.is_valid == YR_FALSE, YR_INVALID);

    disirq = yr_irq_disable();

    /* reset 只允许在没有任务等待队列状态变化时调用，避免唤醒语义混乱 */
    if( !yr_list_isempty( &queue->send_ipc.blocked_list ) ||
        !yr_list_isempty( &queue->receive_ipc.blocked_list ) ) {
        yr_irq_enable(disirq);
        return YR_ERR;
    }

    __queue_reset(queue);

    yr_irq_enable(disirq);

    return YR_OK;
}

yr_err_t yr_queue_send( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks)
{
    yr_uint32_t disirq;
    yr_task_t *current_task;
    yr_bool_t need_switch = YR_FALSE;
    yr_uint32_t start_ticks = 0;
    yr_uint32_t now_ticks;
    yr_uint32_t elapsed_ticks;

    YR_PARAM_CHECK( queue == NULL ||
                    item == NULL, YR_NULL);
    YR_PARAM_CHECK( queue->send_ipc.is_valid == YR_FALSE ||
                    queue->receive_ipc.is_valid == YR_FALSE, YR_INVALID);


    for(;;) {
        disirq = yr_irq_disable();
        current_task = yr_sched_get_current();
        YR_ASSERT( current_task != NULL );

        if( queue->item_count < queue->capacity ) {
            need_switch = YR_FALSE;
            __queue_write_item( queue, item );

            if( !yr_list_isempty( &queue->receive_ipc.blocked_list ) ) {
                __queue_resume_one( &queue->receive_ipc, current_task, &need_switch );
            }

            yr_irq_enable(disirq);

            if( need_switch )
                yr_sched_switch();

            return YR_OK;
        }

        if( wait_ticks == 0 ) {
            yr_irq_enable(disirq);
            return YR_ERR;
        }

        yr_task_set_msg( current_task, (void*)&queue->send_ipc, NULL, YR_TASK_MR_IPC, YR_TASK_MN_NONE );
        yr_ipc_block_task( &queue->send_ipc, current_task );

        if( wait_ticks != YR_WAIT_FOREVER ) {
            if( start_ticks == 0 )
                start_ticks = yr_get_current_ticks();
            yr_timer_set_ticks( &current_task->timer, wait_ticks );
            yr_timer_start( &current_task->timer );
        }

        yr_irq_enable(disirq);
        yr_sched_switch();

        disirq = yr_irq_disable();
        if( current_task->msg_info.notify != YR_TASK_MN_WAIT_OK ) {
            yr_irq_enable(disirq);
            return YR_ERR;
        }
        yr_irq_enable(disirq);

        if( wait_ticks != YR_WAIT_FOREVER ) {
            now_ticks = yr_get_current_ticks();
            elapsed_ticks = now_ticks - start_ticks;
            if( (yr_int32_t)(elapsed_ticks - wait_ticks) >= 0 )
                return YR_ERR;
            wait_ticks -= elapsed_ticks;
            start_ticks = now_ticks;
        }
    }
}

yr_err_t yr_queue_receive( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks)
{
    yr_uint32_t disirq;
    yr_task_t *current_task;
    yr_bool_t need_switch = YR_FALSE;
    yr_uint32_t start_ticks = 0;
    yr_uint32_t now_ticks;
    yr_uint32_t elapsed_ticks;

    YR_PARAM_CHECK( queue == NULL ||
                    item == NULL, YR_NULL);
    YR_PARAM_CHECK( queue->send_ipc.is_valid == YR_FALSE ||
                    queue->receive_ipc.is_valid == YR_FALSE, YR_INVALID);

    for(;;) {
        disirq = yr_irq_disable();
        current_task = yr_sched_get_current();
        YR_ASSERT( current_task != NULL );

        if( queue->item_count > 0 ) {
            need_switch = YR_FALSE;
            __queue_read_item( queue, item );

            if( !yr_list_isempty( &queue->send_ipc.blocked_list ) ) {
                __queue_resume_one( &queue->send_ipc, current_task, &need_switch );
            }

            yr_irq_enable(disirq);

            if( need_switch )
                yr_sched_switch();

            return YR_OK;
        }

        if( wait_ticks == 0 ) {
            yr_irq_enable(disirq);
            return YR_ERR;
        }

        yr_task_set_msg( current_task, (void*)&queue->receive_ipc, NULL, YR_TASK_MR_IPC, YR_TASK_MN_NONE );
        yr_ipc_block_task( &queue->receive_ipc, current_task );

        if( wait_ticks != YR_WAIT_FOREVER ) {
            if( start_ticks == 0 )
                start_ticks = yr_get_current_ticks();
            yr_timer_set_ticks( &current_task->timer, wait_ticks );
            yr_timer_start( &current_task->timer );
        }

        yr_irq_enable(disirq);
        yr_sched_switch();

        disirq = yr_irq_disable();
        if( current_task->msg_info.notify != YR_TASK_MN_WAIT_OK ) {
            yr_irq_enable(disirq);
            return YR_ERR;
        }
        yr_irq_enable(disirq);

        if( wait_ticks != YR_WAIT_FOREVER ) {
            now_ticks = yr_get_current_ticks();
            elapsed_ticks = now_ticks - start_ticks;
            if( (yr_int32_t)(elapsed_ticks - wait_ticks) >= 0 )
                return YR_ERR;
            wait_ticks -= elapsed_ticks;
            start_ticks = now_ticks;
        }
    }
}

static void __queue_reset( yr_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->item_count = 0;
}

static void __queue_write_item( yr_queue_t *queue, const void *item )
{
    yr_uint8_t *dst = queue->buffer + queue->tail * queue->item_size;

    memcpy( dst, item, queue->item_size );

    queue->tail++;
    if( queue->tail >= queue->capacity )
        queue->tail = 0;

    queue->item_count++;
}

static void __queue_read_item( yr_queue_t *queue, void *item )
{
    yr_uint8_t *src = queue->buffer + queue->head * queue->item_size;

    memcpy( item, src, queue->item_size );

    queue->head++;
    if( queue->head >= queue->capacity )
        queue->head = 0;

    queue->item_count--;
}

static yr_err_t __queue_resume_one( yr_ipc_base_t *ipc_base, yr_task_t *current_task, yr_bool_t *need_switch )
{
    yr_task_t *task;

    YR_PARAM_CHECK( ipc_base == NULL, YR_NULL );
    YR_PARAM_CHECK( current_task == NULL, YR_NULL );

    if( yr_list_isempty( &ipc_base->blocked_list ) )
        return YR_OK;

    task = YR_LIST_ENTRY( ipc_base->blocked_list.next, yr_task_t, list_node );
    yr_list_delete_self( &task->list_node );
    yr_timer_stop( &task->timer );

    task->status = YR_TASK_STATUS_READY;
    yr_task_set_msg( task, NULL, NULL, YR_TASK_MR_NONE, YR_TASK_MN_WAIT_OK );
    yr_sched_insert_task( task );

    if( need_switch != NULL &&
        task->current_priority < current_task->current_priority ) {
        *need_switch = YR_TRUE;
    }

    return YR_OK;
}
