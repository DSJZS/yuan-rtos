#ifndef YUAN_RTOS_CONFIG_H
#define YUAN_RTOS_CONFIG_H

/* 时基的频率，表示一秒产生多少 tick  */
#ifndef YR_TICK_RATE_HZ
#define YR_TICK_RATE_HZ                 (1000)
#endif /* YR_TICK_RATE_HZ */

/* 一个时间片默认为多少 tick */
#ifndef YR_DEFAULT_TIME_SLICE_TICKS
#define YR_DEFAULT_TIME_SLICE_TICKS     (10)
#endif /* YR_DEFAULT_TIME_SLICE_TICKS */

/* 开启调试功能 */
#ifndef YR_SUPPORT_DEBUG_LOG
#define YR_SUPPORT_DEBUG_LOG            (1)
#endif /* YR_SUPPORT_DEBUG_LOG */

#if YR_SUPPORT_DEBUG_LOG
/* 调试功能为实现格式化输出需要的缓冲区大小 */
#ifndef YR_PRINTF_BUF_SIZE
#define YR_PRINTF_BUF_SIZE              (128)  
#endif /* YR_PRINTF_BUF_SIZE */
#endif /* YR_SUPPORT_DEBUG_LOG */

/* 开启断言功能 */
#ifndef YR_SUPPORT_ASSERT
#define YR_SUPPORT_ASSERT               (1)
#endif /* YR_SUPPORT_ASSERT */

/* 开启参数检查 */
#ifndef YR_SUPPORT_PARAM_CHECK           
#define YR_SUPPORT_PARAM_CHECK          (1)
#endif /* YR_SUPPORT_PARAM_CHECK */

/* IDLE任务的栈大小 */
#ifndef YR_IDLE_TASK_STACK_SZIE         
#define YR_IDLE_TASK_STACK_SZIE         (256)
#endif /* YR_IDLE_TASK_STACK_SZIE */

/* IPC 是否被支持 */
#ifndef YR_SUPPORT_IPC
#define YR_SUPPORT_IPC                  (1)
#endif /* YR_SUPPORT_IPC */

/* IPC 相关功能 */
#if YR_SUPPORT_IPC

/* 信号量是否被支持 */
#ifndef YR_SUPPORT_SEMAPHORE
#define YR_SUPPORT_SEMAPHORE            (1)
#endif /* YR_SUPPORT_SEMAPHORE */

/* 互斥锁是否被支持 */
#ifndef YR_SUPPORT_MUTEX
#define YR_SUPPORT_MUTEX                (1)
#endif /* YR_SUPPORT_MUTEX */

/* 互斥锁是否被支持 */
#ifndef YR_SUPPORT_QUEUE
#define YR_SUPPORT_QUEUE                (1)
#endif /* YR_SUPPORT_QUEUE */

#endif /* YR_SUPPORT_IPC */

#endif /* YUAN_RTOS_CONFIG_H */
