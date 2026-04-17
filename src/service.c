#include "service.h"

#if YR_SUPPORT_DEBUG_LOG

#ifndef __weak
#define __weak __attribute__((weak))
#endif

/**
 * @brief Weak putc hook (user may override for UART / SWO / etc.).
 */
__weak void yr_putc(char c)
{
    (void)c;
}

/**
 * @brief Very small vsnprintf-like formatter (supports %d %s %c).
 * @param buffer Output buffer.
 * @param size Buffer size (including terminator).
 * @param fmt Format string.
 * @param args Vararg list.
 */
int yr_vsnprintf(char *buffer, size_t size, const char *fmt, va_list args)
{
    char *ptr       = buffer;
    const char *end = buffer + size - 1; /* Preserve space for NUL */

    while (*fmt && ptr < end)
    {
        if (*fmt == '%')
        {
            fmt++;
            if (*fmt == 'd')
            {
                int value = va_arg(args, int);
                char temp[12];
                int  len = 0;

                if (value < 0)
                {
                    if (ptr < end) *ptr++ = '-';
                    value = -value;
                }
                do
                {
                    temp[len++] = (char)('0' + (value % 10));
                    value /= 10;
                } while (value && len < (int)sizeof(temp));

                while (len-- && ptr < end)
                    *ptr++ = temp[len];
            }
            else if (*fmt == 's')
            {
                const char *str = va_arg(args, const char *);
                while (*str && ptr < end)
                    *ptr++ = *str++;
            }
            else if (*fmt == 'c')
            {
                char c = (char)va_arg(args, int);
                if (ptr < end) *ptr++ = c;
            }
            else
            {
                if (ptr < end) *ptr++ = '%';
                if (ptr < end) *ptr++ = *fmt;
            }
        }
        else
        {
            if (ptr < end) *ptr++ = *fmt;
        }
        fmt++;
    }

    *ptr = '\0';
    return (int)(ptr - buffer);
}

/**
 * @brief Lightweight printf forwarding to s_putc().
 */
void yr_printf(const char *fmt, ...)
{
    char buffer[YR_PRINTF_BUF_SIZE];
    va_list args;
    int length;

    va_start(args, fmt);
    length = yr_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    for (int i = 0; i < length; i++)
        yr_putc(buffer[i]);
}

#endif /* YR_SUPPORT_DEBUG_LOG */
