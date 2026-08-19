# 常见问题 FAQ

## 启动后没有日志输出？

1. 确认实现了 `yr_putc()` 并正确初始化了串口
2. 确认 `YR_SUPPORT_DEBUG_LOG` 为 1
3. 确认日志长度未超过 `YR_PRINTF_BUF_SIZE`（默认 128B，超长会被截断）
4. 确认调用 `yr_kernel_init()` 前硬件已初始化

详见 [调试方法](/tutorial/api/debug)。

## 一启动就 HardFault 或进不了首任务？

- 检查 `yr_task_stack_init()` 构造的初始栈帧是否正确
- 检查 PC 是否指向有效的 Thumb 地址、栈顶是否满足对齐要求
- 检查 `SVC_Handler` / `PendSV_Handler` 是否接入了正确的向量表
- 检查任务栈是否过小

## 任务一切换就死机？

- 检查普通任务切换时保存/恢复的寄存器是否完整
- 检查旧任务 SP 是否正确写回、新任务 SP 是否正确读出
- 检查异常返回模式是否正确

## 延时不准或完全不生效？

- 确认 `yr_tick_update()` 真的在周期中断中被调用
- 确认中断频率等于 `YR_TICK_RATE_HZ`
- 周期任务推荐用 `yr_task_sleep_until()` 而不是反复 `yr_task_delay()`

## ISR 唤醒任务后没有切换？

- 确认 `*_from_isr()` 返回的 `need_switch` 为 `YR_TRUE`
- 确认 ISR 末尾调用了 `yr_sched_switch()`
- 确认中断退出路径允许 PendSV（或等价机制）生效

## 日志一开就异常？

- 检查 `yr_putc()` 是否可重入、串口发送是否阻塞过久
- 日志格式化使用栈上 128B 缓冲区，任务栈太小时会溢出
- 先关闭日志（`YR_SUPPORT_DEBUG_LOG` 置 0）可快速判断是否日志导致

## 在中断里能用哪些 API？

信号量、队列提供专门的 `*_isr` 接口；任务管理类 API 通常只能在任务上下文调用。高频 ISR 中尽量避免耗时操作。

## 任务栈开多大合适？

没有固定答案，建议：从 512B 起步，开启断言和日志实测；调用 `yr_printf` 等格式化输出时至少额外预留 128B+ 调用深度。栈溢出常表现为运行一段时间后莫名异常。

## 可以动态创建任务/信号量吗？

Yuan RTOS 采用静态分配模型，任务、信号量等对象由用户提供内存（如 `yr_task_t` + 栈数组）。内核不使用 malloc，需要动态分配时请自行实现内存池/堆。

## 如何减小 RAM/ROM 占用？

- 编译优化使用 `-Os`
- `YR_SUPPORT_DEBUG_LOG`、`YR_SUPPORT_ASSERT`、`YR_SUPPORT_PARAM_CHECK` 置 0
- 关闭不需要的 IPC 模块（信号量/互斥锁/队列）
- 减小 `YR_PRINTF_BUF_SIZE`

## 支持哪些内核与编译器？

官方移植：Cortex-M3 + GCC（`libcpu/CM3/port_gcc.c/.s`）；BSP 提供 STM32F103C8T6 的 CMake 与 EWARM 工程。其它架构需自行移植。

## 如何移植到其它芯片？

移植分两部分：实现 `portable.h` 声明的接口（中断开关、任务栈初始化、首任务启动、任务切换、查找最低位 1），并提供匹配 `YR_TICK_RATE_HZ` 的 tick 中断。完整步骤见 [移植指南](/tutorial/porting/)。

## 报告问题需要提供什么？

平台、编译器与优化级别、`yr_config.h` 是否修改、复现步骤，以及能的话附上日志/map 文件。
