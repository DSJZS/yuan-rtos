# 软件定时器

## 获取当前 tick

```c
yr_uint32_t yr_get_current_ticks(void);
```

获取当前系统 tick 计数值。

- 返回值
  - 当前 tick 值
- 说明
  - 返回值会随系统节拍持续递增
  - 该函数本身不是严格原子接口，若应用对一致性要求极高，可自行在临界区内读取

## 初始化定时器链表

```c
void yr_timer_list_init(void);
```

初始化内核的软件定时器链表。

- 行为
  - 将内部定时器链表恢复为空链表状态
- 说明
  - 该函数一般由 `yr_kernel_init()` 内部调用
  - 应用层通常不需要手动调用

## 定时器初始化

```c
yr_err_t yr_timer_init( yr_timer_t *timer, yr_timer_func_t func, void *param, yr_uint32_t ticks);
```

初始化一个软件定时器对象。

- 参数
  - timer 定时器对象指针
  - func 超时回调函数，函数类型为 `void (*)(void *)`
  - param 传递给超时回调的参数
  - ticks 定时器的初始 tick 周期
- 行为
  - 初始化定时器内部链表节点
  - 保存超时回调、参数和初始周期
  - 此时仅完成对象初始化，不会自动开始计时
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - `timer` 与 `func` 不能为空
  - 建议定时器对象使用静态/全局存储，避免超时前对象失效

## 启动定时器

```c
yr_err_t yr_timer_start( yr_timer_t *timer);
```

启动一个已经初始化的软件定时器。

- 参数
  - timer 定时器对象指针
- 行为
  - 若该定时器之前已经在链表中，会先移除旧节点再重新启动
  - 超时时刻按“当前 tick + init_ticks”计算
  - 定时器会被插入到按超时时刻升序排列的定时器链表中
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 重新调用该函数可实现“重新装载/重启定时器”的效果

## 停止定时器

```c
yr_err_t yr_timer_stop( yr_timer_t *timer);
```

停止一个软件定时器。

- 参数
  - timer 定时器对象指针
- 行为
  - 将该定时器从内核定时器链表中移除
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 停止后不会触发超时回调，除非之后再次调用[启动定时器](#启动定时器)

## 设置定时器周期

```c
yr_err_t yr_timer_set_ticks( yr_timer_t *timer, yr_uint32_t ticks);
```

设置软件定时器的周期 tick 数。

- 参数
  - timer 定时器对象指针
  - ticks 新的周期值
- 行为
  - 仅修改 `init_ticks`
  - 不会自动重排已启动定时器在链表中的位置
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 若定时器已经在运行，通常应在设置后重新调用[启动定时器](#启动定时器)，使新周期立即生效

## 系统 tick 更新

```c
void yr_tick_update(void);
```

每来一次系统节拍时调用一次，用于推动整个时间系统向前运行。

- 行为
  - 全局 tick 自增
  - 递减当前运行任务的剩余时间片
  - 若当前任务时间片耗尽，则触发同优先级轮转调度
  - 检查定时器链表，将已超时的定时器取出并执行对应回调
- 说明
  - 该函数应在系统节拍中断中按 `YR_TICK_RATE_HZ` 频率调用
  - 这是任务延时、IPC 超时等待、软件定时器超时生效的基础

## 默认超时回调

```c
void yr_timeout_default_func(void *param);
```

默认的超时回调函数，主要供任务延时和 IPC 超时等待使用。

- 参数
  - param 一般为任务对象指针
- 行为
  - 若对应任务当前仍处于 `BLOCKED` 状态，则将其恢复为 `READY`
  - 设置任务唤醒通知为 `YR_TASK_MN_WAIT_TIMEOUT`
  - 将任务重新加入调度器
  - 若其优先级高于当前运行任务，则可能触发一次调度
- 说明
  - 该函数通常由内核内部使用，应用层一般不直接调用

## 软件定时器示例

```c
static yr_timer_t led_timer;

static void led_timeout(void *param)
{
    (void)param;
    led_toggle();

    /* 重新启动，形成周期软件定时器 */
    yr_timer_start(&led_timer);
}

void app_timer_init(void)
{
    yr_timer_init(&led_timer, led_timeout, NULL, YR_MS_TO_TICKS(500));
    yr_timer_start(&led_timer);
}
```

