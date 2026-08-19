# 移植收尾

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

