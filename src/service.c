#include "service.h"

#if YR_SUPPORT_DEBUG_LOG

#ifndef __weak
#define __weak __attribute__((weak))
#endif

/**
 * @brief 弱定义的字符输出钩子，用户可在 BSP 中重写。
 * @param c 待输出字符。
 */
__weak void yr_putc(char c)
{
    (void)c;
}

/**
 * @brief 轻量级格式化函数，支持 %d %u %x %s %c。
 * @param buffer 输出缓冲区。
 * @param size 缓冲区大小，包含结尾的 '\0'。
 * @param fmt 格式字符串。
 * @param args 可变参数列表。
 * @return 实际写入的字符数，不包含结尾的 '\0'。
 */
int yr_vsnprintf(char *buffer, size_t size, const char *fmt, va_list args)
{
    char *ptr       = buffer;
    const char *end = buffer + size - 1; /* 预留字符串结尾的 '\0' */

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
            else if (*fmt == 'u')
            {
                unsigned int value = va_arg(args, unsigned int);
                char temp[12];
                int  len = 0;

                do
                {
                    temp[len++] = (char)('0' + (value % 10U));
                    value /= 10U;
                } while (value && len < (int)sizeof(temp));

                while (len-- && ptr < end)
                    *ptr++ = temp[len];
            }
            else if (*fmt == 'x')
            {
                unsigned int value = va_arg(args, unsigned int);
                char temp[8];
                int  len = 0;

                do
                {
                    unsigned int digit = value & 0xFU;
                    temp[len++] = (char)(digit < 10U ? ('0' + digit) : ('a' + digit - 10U));
                    value >>= 4;
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
 * @brief 轻量级 printf，逐字符转发到 yr_putc。
 * @param fmt 格式字符串。
 * @param ... 可变参数列表。
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
