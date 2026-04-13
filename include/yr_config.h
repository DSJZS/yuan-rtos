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
#ifndef YR_DEBUG_LOG_ON
#define YR_DEBUG_LOG_ON                 (1)
#endif /* YR_DEBUG_LOG_ON */

#if YR_DEBUG_LOG_ON
/* 调试功能为实现格式化输出需要的缓冲区大小 */
#ifndef YR_PRINTF_BUF_SIZE
#define YR_PRINTF_BUF_SIZE              (128)  
#endif /* YR_PRINTF_BUF_SIZE */
#endif /* YR_DEBUG_LOG_ON */

/* 开启断言功能 */
#ifndef YR_ASSERT_ON
#define YR_ASSERT_ON                    (1)
#endif /* YR_ASSERT_ON */

/* 开启参数检查 */
#ifndef YR_PARAM_CHECK_ON           
#define YR_PARAM_CHECK_ON               (1)
#endif /* YR_PARAM_CHECK_ON */

#endif /* YUAN_RTOS_CONFIG_H */
