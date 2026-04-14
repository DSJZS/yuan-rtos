#ifndef YUAN_RTOS_DEF_H
#define YUAN_RTOS_DEF_H

#include "yr_config.h"
#include "service.h"
#include "portable.h"

/* 就绪 */
#define YR_TASK_STATUS_READY        0x01
/* 运行 */
#define YR_TASK_STATUS_RUNNING      0x02
/* 阻塞 */
#define YR_TASK_STATUS_BLOCKED      0x04
/* 挂起(暂停) */
#define YR_TASK_STATUS_SUSPENDED    0x08
/* 终止(待删除，僵尸任务) */
#define YR_TASK_STATUS_TERMINATED   0x10
/* 删除(不再被管理) */
#define YR_TASK_STATUS_DELETED      0x20

typedef enum yr_err_t {
    YR_OK = 0,
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

/* 日志输出 */
#if YR_DEBUG_LOG_ON
#define YR_DEBUG_LOG(level, fmt, ...) do {                                      \
    const char *log_level =                                                     \
        ((level) == YR_DEBUG_ERROR)  ? "ERR" :                                  \
        ((level) == YR_DEBUG_WARN) ? "WARN" : "INFO";                           \
    yr_printf("[%s] " fmt, log_level, ##__VA_ARGS__);                           \
} while (0)
#else
#define YR_DEBUG_LOG(level, fmt, ...) ((void)0)
#endif /* YR_DEBUG_LOG_ON */

/* 断言判断 */
#if YR_ASSERT_ON
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
#endif /* YR_ASSERT_ON */

/* 参数检查 */
#if YR_PARAM_CHECK_ON
#define YR_PARAM_CHECK( expr, ret) do {                                         \
    if( (expr) ) {                                                              \
        return ret;                                                             \
    }                                                                           \
} while(0)
#else
#define YR_PARAM_CHECK( expr, ret) ((void)0)
#endif /* YR_PARAM_CHECK_ON */

#endif /* YUAN_RTOS_DEF_H */
