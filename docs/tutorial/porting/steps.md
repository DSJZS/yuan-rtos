# 推荐的移植步骤

## 第一步：先让工程能编译通过

- 建立新的 `libcpu/<arch>/port_<compiler>.c/.s`
- 修改构建系统，使其能编译进工程
- 先把 `portable.h` 中接口全部实现出来，即使一开始只是最小空实现

## 第二步：实现中断开关接口

优先完成：

- `yr_irq_disable()`
- `yr_irq_enable()`

因为后续任务、调度器、定时器很多逻辑都依赖临界区保护。

## 第三步：实现 `yr_find_first_set()`

这个接口通常很简单，优先做掉，方便调度器先跑通。

## 第四步：实现任务栈初始化

完成：

- `yr_task_stack_init()`

并重点确认：

- 参数是否正确进入任务入口
- 栈对齐是否满足 ABI 要求
- 任务返回路径是否能落到 `__task_exit`

## 第五步：实现普通任务切换

完成：

- `yr_task_switch()`
- 对应的异常/汇编切换处理逻辑

目标是让任务之间可以正常切换。

## 第六步：实现首任务启动

完成：

- `yr_task_first_switch_to()`

目标是让 `yr_kernel_start()` 能真正切进第一个任务。

## 第七步：接入 tick 中断

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

