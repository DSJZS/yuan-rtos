#ifndef YUAN_RTOS_DEF_H
#define YUAN_RTOS_DEF_H

#include "yr_config.h"
#include "service.h"
#include "portable.h"

typedef enum yr_err_t {
    YR_OK = 0,
    YR_ERR,
    YR_NULL,
    YR_INVALID,
} yr_err_t;

typedef enum yr_bool_t {
    YR_FALSE = 0,
    YR_TRUE,
} yr_bool_t;

typedef enum yr_log_level_t {
    YR_DEBUG_ERROR = 0,
    YR_DEBUG_WARN,
    YR_DEBUG_INFO,
} yr_log_level_t;

/* 类似 linux 内核的 container_of 操作 */
#define YR_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define YR_TICKS_TO_MS(ticks) \
    ((yr_uint32_t)((ticks) * 1000U / YR_TICK_RATE_HZ))

#define YR_MS_TO_TICKS(ms) \
    ((yr_uint32_t)(((ms) * YR_TICK_RATE_HZ + 999U) / 1000U))


#define YR_WAIT_FOREVER     (yr_uint32_t)(0xFFFFFFFFU)

/* 日志输出 */
#if YR_SUPPORT_DEBUG_LOG
#define YR_DEBUG_LOG(level, fmt, ...) do {                                      \
    const char *log_level =                                                     \
        ((level) == YR_DEBUG_ERROR)  ? "ERR" :                                  \
        ((level) == YR_DEBUG_WARN) ? "WARN" : "INFO";                           \
    yr_printf("[%s] " fmt, log_level, ##__VA_ARGS__);                           \
} while (0)
#else
#define YR_DEBUG_LOG(level, fmt, ...) ((void)0)
#endif /* YR_SUPPORT_DEBUG_LOG */

/* 断言判断 */
#if YR_SUPPORT_ASSERT
#define YR_ASSERT(expr) do {                                                    \
    if( !(expr) ) {                                                             \
        (void)yr_irq_disable();                                                 \
        YR_DEBUG_LOG(YR_DEBUG_ERROR, "assert fail, %s( line %d ): %s\r\n",      \
                                                __func__, __LINE__, #expr);     \
        while(1);                                                               \
    }                                                                           \
} while (0)                                                                     
#else
#define YR_ASSERT(expr) ((void)0)
#endif /* YR_SUPPORT_ASSERT */

/* 参数检查 */
#if YR_SUPPORT_PARAM_CHECK
#define YR_RETURN_NONE
#define YR_PARAM_CHECK( expr, ret) do {                                         \
    if( (expr) ) {                                                              \
        return ret;                                                             \
    }                                                                           \
} while(0)
#else
#define YR_PARAM_CHECK( expr, ret) ((void)0)
#endif /* YR_SUPPORT_PARAM_CHECK */

#endif /* YUAN_RTOS_DEF_H */
