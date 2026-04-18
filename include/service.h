#ifndef YUAN_RTOS_SERVICE_H
#define YUAN_RTOS_SERVICE_H

#include "yr_config.h"

#if YR_SUPPORT_DEBUG_LOG

#include <stdarg.h>
#include <stddef.h>

/**
 * @brief 输出格式化日志(支持 %d %u %x %s %c)。
 * @param fmt 格式化字符串。
 * @param ... 可变参数列表。
 */
void yr_printf(const char *fmt, ...);

#endif /* YR_SUPPORT_DEBUG_LOG */

#endif /* YUAN_RTOS_SERVICE_H */
