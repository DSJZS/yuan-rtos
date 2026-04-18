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

/**
 * @brief 关闭常规中断并保存当前中断状态。
 * @return 关闭前的中断状态。
 */
yr_uint32_t yr_irq_disable(void);

/**
 * @brief 恢复中断状态。
 * @param disirq 先前保存的中断状态。
 */
void yr_irq_enable(yr_uint32_t disirq);

/**
 * @brief 初始化任务首次运行所需的栈帧。
 * @param entry 任务入口函数。
 * @param exit 任务退出时跳转的函数。
 * @param param 传递给任务入口函数的参数。
 * @param stackaddr 栈顶地址。
 * @return 初始化后的 SP 值。
 */
yr_uint8_t *yr_task_stack_init( void *entry, void *exit,  void *param, yr_uint8_t *stackaddr);

/**
 * @brief 启动调度并切换到第一个任务。
 * @param to 第一个任务栈指针变量的地址。
 */
void yr_task_first_switch_to( yr_uint32_t to);

/**
 * @brief 请求由当前任务切换到目标任务。
 * @param from 当前任务栈指针变量的地址。
 * @param to 目标任务栈指针变量的地址。
 */
void yr_task_switch( yr_uint32_t from, yr_uint32_t to);

/**
 * @brief 查找整数最低位的置位位置。
 * @param value 输入值。
 * @return 最低位 1 的位序号，若没有置位则返回 0。
 */
int yr_find_first_set(int value);

#endif /* YUAN_RTOS_PORTABLE_H */
