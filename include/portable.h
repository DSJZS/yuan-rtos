#ifndef YUAN_RTOS_PORTABLE_H
#define YUAN_RTOS_PORTABLE_H

#include <stdint.h>
#include <stddef.h>

/* 常用数据类型定义 */
typedef uint8_t         yr_uint8_t;
typedef uint16_t        yr_uint16_t;
typedef uint32_t        yr_uint32_t;
typedef uint64_t        yr_uint64_t;
typedef int8_t          yr_int8_t;
typedef int16_t         yr_int16_t;
typedef int32_t         yr_int32_t;
typedef int64_t         yr_int64_t;

// void port_task_switch_to( task_handler_t to);
// void port_task_switch( task_handler_t from, task_handler_t to);

/* 关闭常规中断(对于单核处理器相当于进入临界区) */
yr_uint32_t yr_irq_disable(void);
/* 打开常规中断(对于单核处理器相当于退出临界区) */
void yr_irq_enable(yr_uint32_t disirq);

/* 任务栈初始化 */
yr_uint8_t *yr_task_stack_init( void *entry, void *exit,  void *param, yr_uint8_t *stackaddr);
/* 运行后切换到第一个任务 */
void yr_task_first_switch_to( yr_uint32_t to);
/* 由一个任务切换到另一个任务 */
void yr_task_switch( yr_uint32_t from, yr_uint32_t to);
/* 接收一个32位的数据，返回被置1的最低位的位数，比如输入 0x01 会返回 1，如果 0x00，返回0*/
int yr_find_first_set(int value);

#endif /* YUAN_RTOS_PORTABLE_H */
