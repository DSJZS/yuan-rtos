# 系统 tick 应该怎么接

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
