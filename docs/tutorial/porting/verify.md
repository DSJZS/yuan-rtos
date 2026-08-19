# 推荐的最小验证步骤

## 验证 1：单任务启动

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

## 验证 2：两个同优先级任务轮转

创建两个相同优先级任务，并让它们周期打印不同内容。

如果能交替输出，说明：

- 时间片轮转工作正常
- `yr_task_switch()` 基本正确

## 验证 3：任务延时

让任务调用：

```c
yr_task_sleep_ticks(YR_MS_TO_TICKS(1000));
```

如果能按预期频率运行，说明：

- tick 中断频率正确
- `yr_tick_update()` 已接入
- 定时器链表工作正常

## 验证 4：信号量/队列

最后再验证：

- 信号量同步
- 队列收发
- ISR 到任务的唤醒路径

如果这些都正常，说明移植已基本完成。

