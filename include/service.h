#ifndef YUAN_RTOS_SERVICE_H
#define YUAN_RTOS_SERVICE_H

#include "yr_config.h"

#if YR_SUPPORT_DEBUG_LOG

#include <stdarg.h>
#include <stddef.h>

void yr_printf(const char *fmt, ...);

#endif /* YR_SUPPORT_DEBUG_LOG */

#endif /* YUAN_RTOS_SERVICE_H */
