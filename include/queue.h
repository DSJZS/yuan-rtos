#ifndef YUAN_RTOS_QUEUE_H
#define YUAN_RTOS_QUEUE_H

#include "yr_config.h"

#if YR_SUPPORT_QUEUE

#include "ipc.h"
#include "portable.h"
#include "yr_def.h"

typedef struct yr_queue_t {
    yr_ipc_base_t send_ipc;
    yr_ipc_base_t receive_ipc;

    yr_uint8_t *buffer;
    yr_uint32_t capacity;
    yr_uint32_t item_size;

    yr_uint32_t item_count;
    yr_uint32_t head;
    yr_uint32_t tail;
} yr_queue_t;

/**
 * @brief 初始化消息队列。
 * @param queue 队列对象指针。
 * @param item_size 单个元素大小。
 * @param buffer 队列存储缓冲区。
 * @param buffer_size 缓冲区总大小。
 * @param flag 阻塞任务的排队策略。
 * @return 初始化结果。
 */
yr_err_t yr_queue_init( yr_queue_t *queue, yr_uint32_t item_size, yr_uint8_t *buffer, yr_uint32_t buffer_size, yr_uint32_t flag);

/**
 * @brief 删除消息队列并唤醒等待任务。
 * @param queue 队列对象指针。
 * @return 操作结果。
 */
yr_err_t yr_queue_delete( yr_queue_t *queue);

/**
 * @brief 重置消息队列内容。
 * @param queue 队列对象指针。
 * @return 操作结果。
 */
yr_err_t yr_queue_reset( yr_queue_t *queue);

/**
 * @brief 向消息队列发送一个元素。
 * @param queue 队列对象指针。
 * @param item 待发送数据的地址。
 * @param wait_ticks 队列满时允许等待的 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_queue_send( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);

/**
 * @brief 从消息队列接收一个元素。
 * @param queue 队列对象指针。
 * @param item 接收缓冲区地址。
 * @param wait_ticks 队列空时允许等待的 tick 数。
 * @return 操作结果。
 */
yr_err_t yr_queue_receive( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);

/**
 * @brief 在中断中向消息队列发送一个元素。
 * @param queue 队列对象指针。
 * @param item 待发送数据的地址。
 * @param need_switch 若唤醒了更高优先级任务则置为 YR_TRUE。
 * @return 操作结果。
 */
yr_err_t yr_queue_send_from_isr( yr_queue_t *queue, void *item, yr_bool_t *need_switch);

/**
 * @brief 在中断中从消息队列接收一个元素。
 * @param queue 队列对象指针。
 * @param item 接收缓冲区地址。
 * @param need_switch 若唤醒了更高优先级任务则置为 YR_TRUE。
 * @return 操作结果。
 */
yr_err_t yr_queue_receive_from_isr( yr_queue_t *queue, void *item, yr_bool_t *need_switch);

#endif /* YR_SUPPORT_QUEUE */

#endif /* YUAN_RTOS_QUEUE_H */
