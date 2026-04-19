![Yuan](images/YuanLogo.jpg)  
# Yuan RTOS 移植指南

本文档说明如何将 Yuan RTOS 移植到新的 MCU、CPU 架构或编译器环境。

目标是回答下面几个问题：

- 需要改哪些文件
- 必须实现哪些底层接口
- 系统 tick 应该怎么接
- 第一次跑起来后应该怎么验证

## 适用范围

本文档主要面向以下几类场景：

- 将 Yuan RTOS 从当前 `Cortex-M3 + GCC` 移植到其它 Cortex-M 内核
- 将 Yuan RTOS 移植到不同编译器
- 将 Yuan RTOS 移植到全新架构

如果你的目标平台与当前仓库中的 `libcpu/CM3/port_gcc.c`、`libcpu/CM3/port_gcc.s` 很接近，建议优先在现有实现基础上修改，而不是从零开始。

## 先理解 Yuan RTOS 的移植边界

Yuan RTOS 的整体结构可以分为两层：

- 通用内核层
  - 位于 `src/` 和 `include/`
  - 包含任务管理、调度器、定时器、IPC 等逻辑
- 移植层
  - 位于 `libcpu/`
  - 负责中断控制、上下文切换、任务初始栈帧构造、首任务启动等与平台强相关的内容

也就是说，移植时你通常不需要改任务、调度器、IPC 的核心逻辑，而是要让这些通用逻辑能正确调用到底层 CPU 机制。

## 移植时必须完成的两件事

移植 Yuan RTOS 到新平台时，必须完成以下两部分：

1. 实现 `include/portable.h` 中声明的移植层接口
2. 提供满足 `YR_TICK_RATE_HZ` 的系统 tick，并在其中调用 `yr_tick_update()`

只要这两部分正确接入，RTOS 的大部分通用能力就可以运行起来。

## 推荐的目录组织

建议按下面的方式组织移植层文件：

```text
libcpu/<ISA>/port_<compiler>.c
libcpu/<ISA>/port_<compiler>.s
```

例如：

- `libcpu/CM3/port_gcc.c`
- `libcpu/CM3/port_gcc.s`

如果你要移植到 Cortex-M4 + GCC，可以参考：

```text
libcpu/CM4/port_gcc.c
libcpu/CM4/port_gcc.s
```

如果你要移植到 Cortex-M3 + ARMCC，也可以参考：

```text
libcpu/CM3/port_armcc.c
libcpu/CM3/port_armcc.s
```

## 需要实现的移植接口

Yuan RTOS 当前要求实现下面几个接口，这些接口都定义在 `include/portable.h` 中。

### 1. `yr_irq_disable()`

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

### 2. `yr_irq_enable()`

```c
void yr_irq_enable(yr_uint32_t disirq);
```

作用：

- 恢复先前保存的中断状态

要求：

- 必须与 `yr_irq_disable()` 成对使用
- 恢复后系统中断状态应回到进入临界区之前

### 3. `yr_task_stack_init()`

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

### 4. `yr_task_first_switch_to()`

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

### 5. `yr_task_switch()`

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

### 6. `yr_find_first_set()`

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

## 任务切换机制应该怎么实现

不同架构的实现方式可以不同，但总体要满足下面这条逻辑链：

1. 调度器选出下一个要运行的任务
2. 调用 `yr_task_switch()`
3. 底层保存当前任务上下文
4. 底层恢复目标任务上下文
5. CPU 回到线程模式继续执行

对于 Cortex-M，一般建议：

- 使用 `PendSV` 承担普通任务切换
- 使用 `SVC` 承担首任务启动
- 使用 `PSP` 作为任务栈
- 将 `PendSV` 设置为最低优先级，避免破坏中断实时性

如果你移植到非 Cortex-M 架构，可以不使用 `SVC/PendSV`，但要保证：

- 普通任务切换可重入性正确
- 切换原子性正确
- 任务上下文保存/恢复完整

## 需要保存哪些寄存器

这取决于目标架构 ABI 和异常模型，但原则上要保证：

- 任务被切出前，它的运行现场能完整保存
- 任务被切回时，能从上次停下的位置继续执行

以当前 Cortex-M3 实现为例：

- 硬件异常进入时自动压栈：
  - `r0-r3`
  - `r12`
  - `lr`
  - `pc`
  - `xPSR`
- 软件额外保存：
  - `r4-r11`

所以移植到新架构时，你首先要搞清楚：

- 哪些寄存器由硬件自动保存
- 哪些寄存器必须由软件自己保存
- 线程栈与异常栈是否分离
- 返回线程模式时需要恢复哪些特殊状态位

## 系统 tick 应该怎么接

除了上下文切换外，RTOS 还依赖周期 tick 驱动时间系统。

你必须提供一个周期中断，其频率满足：

```c
YR_TICK_RATE_HZ
```

并在该中断中调用：

```c
yr_tick_update();
```

典型形式如下：

```c
void SysTick_Handler(void)
{
    /* 平台相关前置代码 */
    yr_tick_update();
    /* 平台相关后置代码 */
}
```

作用：

- 推进全局 tick
- 更新任务时间片
- 处理软件定时器
- 处理任务超时唤醒

注意事项：

- tick 中断频率必须和 `YR_TICK_RATE_HZ` 一致
- 若 tick 频率配置错误，任务延时、超时和时间片都会失真

## 需要用户在 BSP 中补充的内容

完成移植后，通常还需要在板级工程中补足下面这些能力：

### 1. 时钟和基础硬件初始化

至少要保证：

- CPU 时钟已经稳定
- RAM 可正常使用
- 中断系统已可用

### 2. 系统 tick 源

通常使用：

- `SysTick`
- 通用定时器中断
- 平台提供的系统时基

### 3. 日志输出接口（可选）

如果开启了：

```c
#define YR_SUPPORT_DEBUG_LOG    (1)
```

则建议实现：

```c
void yr_putc(char c);
```

例如通过 UART 输出单字符。

### 4. 启动文件 / 向量表配置

如果你的移植实现依赖：

- `SVC_Handler`
- `PendSV_Handler`
- `SysTick_Handler`

那么必须保证这些异常入口已正确接入启动文件和向量表。

## 推荐的移植步骤

建议按下面顺序推进，这样最不容易卡住。

### 第一步：先让工程能编译通过

- 建立新的 `libcpu/<arch>/port_<compiler>.c/.s`
- 修改构建系统，使其能编译进工程
- 先把 `portable.h` 中接口全部实现出来，即使一开始只是最小空实现

### 第二步：实现中断开关接口

优先完成：

- `yr_irq_disable()`
- `yr_irq_enable()`

因为后续任务、调度器、定时器很多逻辑都依赖临界区保护。

### 第三步：实现 `yr_find_first_set()`

这个接口通常很简单，优先做掉，方便调度器先跑通。

### 第四步：实现任务栈初始化

完成：

- `yr_task_stack_init()`

并重点确认：

- 参数是否正确进入任务入口
- 栈对齐是否满足 ABI 要求
- 任务返回路径是否能落到 `__task_exit`

### 第五步：实现普通任务切换

完成：

- `yr_task_switch()`
- 对应的异常/汇编切换处理逻辑

目标是让任务之间可以正常切换。

### 第六步：实现首任务启动

完成：

- `yr_task_first_switch_to()`

目标是让 `yr_kernel_start()` 能真正切进第一个任务。

### 第七步：接入 tick 中断

在定时中断里调用：

```c
yr_tick_update();
```

目标是让：

- `yr_task_sleep_ticks()`
- `yr_task_sleep_until()`
- 软件定时器
- 时间片轮转

全部工作起来。

## 推荐的最小验证步骤

不要一上来就测 IPC。建议按下面顺序逐层验证。

### 验证 1：单任务启动

创建一个任务，让它进入死循环并翻转 GPIO 或打印日志：

```c
static yr_task_t task0;
static yr_uint8_t task0_stack[512];

static void task0_entry(void *param)
{
    (void)param;

    for(;;) {
        led_toggle();
    }
}
```

若该任务能成功运行，说明：

- 首任务启动基本正确
- 初始栈帧基本正确

### 验证 2：两个同优先级任务轮转

创建两个相同优先级任务，并让它们周期打印不同内容。

如果能交替输出，说明：

- 时间片轮转工作正常
- `yr_task_switch()` 基本正确

### 验证 3：任务延时

让任务调用：

```c
yr_task_sleep_ticks(YR_MS_TO_TICKS(1000));
```

如果能按预期频率运行，说明：

- tick 中断频率正确
- `yr_tick_update()` 已接入
- 定时器链表工作正常

### 验证 4：信号量/队列

最后再验证：

- 信号量同步
- 队列收发
- ISR 到任务的唤醒路径

如果这些都正常，说明移植已基本完成。

## 常见错误与排查建议

### 1. 一启动就 HardFault

优先检查：

- `yr_task_stack_init()` 构造的初始栈帧是否正确
- PC 是否指向有效 Thumb 地址
- 栈顶是否满足对齐要求
- `SVC_Handler` / `PendSV_Handler` 是否接入了正确的向量表

### 2. 能进首任务，但一切换就死机

优先检查：

- 普通任务切换时保存/恢复的寄存器是否完整
- 旧任务 SP 是否正确写回
- 新任务 SP 是否正确读出
- 异常返回模式是否正确

### 3. 延时不准或完全不生效

优先检查：

- `yr_tick_update()` 是否真的在周期中断中被调用
- 调用频率是否等于 `YR_TICK_RATE_HZ`
- 时钟源是否稳定

### 4. ISR 唤醒任务后没有切换

优先检查：

- `*_from_isr()` 返回的 `need_switch` 是否为 `YR_TRUE`
- ISR 末尾是否调用了 `yr_sched_switch()`
- 中断退出路径是否允许 `PendSV` 或等价切换机制生效

### 5. 日志一开就异常

优先检查：

- `yr_putc()` 是否可重入
- 串口发送是否阻塞过久
- 任务栈是否过小

## 移植完成后的建议

移植跑通后，建议再做下面几件事：

- 根据平台实际情况调整 `YR_TICK_RATE_HZ`
- 根据 RAM 情况评估任务栈大小
- 如果资源紧张，关闭：
  - `YR_SUPPORT_DEBUG_LOG`
  - `YR_SUPPORT_ASSERT`
  - `YR_SUPPORT_PARAM_CHECK`
- 用 `-Os` 和 `-O2/-O3` 分别测试一次，确认不同优化等级下行为一致

## 与现有移植实现对照阅读

如果你想对照当前仓库中的参考实现，推荐按下面顺序阅读：

1. `include/portable.h`
2. `libcpu/CM3/port_gcc.c`
3. `libcpu/CM3/port_gcc.s`
4. `src/scheduler.c`
5. `src/task.c`
6. `src/timer.c`

这样更容易把“调度器在上层发出切换请求”和“底层如何真正完成切换”连起来看懂。

## 最后建议

Yuan RTOS 的移植工作量并不大，但对 CPU 异常模型、栈布局和 ABI 的要求非常严格。

一个实用的原则是：

- 先跑通单任务
- 再跑通任务切换
- 再跑通 tick
- 最后再测 IPC

不要一开始就同时验证所有模块，这样最容易定位问题。
