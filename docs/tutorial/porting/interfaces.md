# 需要实现的移植接口

## 1. `yr_irq_disable()`

```c
yr_uint32_t yr_irq_disable(void);
```

作用：

- 关闭常规中断
- 返回关闭前的中断状态

要求：

- 返回值必须能被 `yr_irq_enable()` 正确恢复
- 需要保证进入临界区后内核关键数据不会被中断打断

当前 `Cortex-M3 + GCC` 参考实现：

- 使用 `PRIMASK` 保存当前中断屏蔽状态
- 使用 `cpsid i` 关闭 IRQ

## 2. `yr_irq_enable()`

```c
void yr_irq_enable(yr_uint32_t disirq);
```

作用：

- 恢复先前保存的中断状态

要求：

- 必须与 `yr_irq_disable()` 成对使用
- 恢复后系统中断状态应回到进入临界区之前

## 3. `yr_task_stack_init()`

```c
yr_uint8_t *yr_task_stack_init(void *entry, void *exit, void *param, yr_uint8_t *stackaddr);
```

作用：

- 构造任务第一次运行时的初始栈帧

这是整个移植中最关键的接口之一，因为调度器并不会“直接调用任务函数”，而是通过恢复一份预先伪造好的上下文，让任务像被 CPU 正常切入一样开始运行。

要求：

- 栈顶需要按目标架构 ABI 要求对齐
- 初始上下文必须能让任务入口函数收到 `param`
- 任务函数返回时必须跳转到 `exit`
- 必须保证任务第一次恢复时 PC、SP、状态寄存器等内容合法

当前 `Cortex-M3 + GCC` 的做法：

- 对栈顶做 8 字节对齐
- 预留一份 `yr_context_t`
- 将：
  - `r0 = param`
  - `pc = entry`
  - `lr = exit`
  - `xPSR` 置为 Thumb 有效值

## 4. `yr_task_first_switch_to()`

```c
void yr_task_first_switch_to(yr_uint32_t to);
```

作用：

- 在调度器启动后，切换到第一个任务

这个接口和普通任务切换不同，它完成的是“从启动环境进入任务环境”的第一次过渡。

要求：

- 能从 `main()` 或启动后的初始上下文进入线程调度环境
- 能使用首任务的 SP 正确恢复寄存器现场

当前 `Cortex-M3 + GCC` 的做法：

- 将 `PendSV` 配置为最低优先级
- 开中断
- 触发 `SVC`
- 在 `SVC_Handler` 中恢复首任务上下文并切换到 PSP

## 5. `yr_task_switch()`

```c
void yr_task_switch(yr_uint32_t from, yr_uint32_t to);
```

作用：

- 请求从当前任务切换到目标任务

要求：

- 该函数不一定要“当场完成切换”
- 但必须能把旧任务和新任务的 SP 地址记录下来，并触发后续上下文切换流程

当前 `Cortex-M3 + GCC` 的做法：

- 保存：
  - 旧任务 `sp` 地址
  - 新任务 `sp` 地址
- 设置切换标志
- 触发 `PendSV`
- 最终在 `PendSV_Handler` 中完成真正上下文切换

## 6. `yr_find_first_set()`

```c
int yr_find_first_set(int value);
```

作用：

- 查找一个整数最低位 1 的位置

调度器依赖它来快速从优先级位图中找到当前最高优先级就绪任务。

要求：

- 输入为 0 时返回 0
- 其它情况返回最低位 1 的位序号

当前 `Cortex-M3 + GCC` 的做法：

- 直接使用 `__builtin_ffs()`

