#ifndef YUAN_RTOS_KERNEL_H
#define YUAN_RTOS_KERNEL_H

/**
 * @brief 初始化 Yuan RTOS 内核。
 */
void yr_kernel_init(void);

/**
 * @brief 启动 Yuan RTOS 内核，开始调度。
 */
void yr_kernel_start(void);

#endif /* YUAN_RTOS_KERNEL_H */
