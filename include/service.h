#ifndef YUAN_RTOS_SERVICE_H
#define YUAN_RTOS_SERVICE_H

#include "yr_config.h"

#if YR_DEBUG_LOG_ON

#include <stdarg.h>
#include <stddef.h>

void yr_printf(const char *fmt, ...);

#endif /* YR_DEBUG_LOG_ON */

#endif /* YUAN_RTOS_SERVICE_H */
