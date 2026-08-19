# 任务管理

## 任务初始化

```c
yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, 
    void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority, yr_uint32_t ticks);
```

用于初始化一个任务。注意这个函数不会将任务加入调度队列之中。

- 参数
  - task 任务对象指针（任务对象必须静态/全局存储）
  - entry 任务入口函数，函数类型必须为 yr_task_func_t ，原型为 `void (*)(void*)`
  - param 任务入口函数参数
  - stack_addr 栈空间基地址（传入首地址，内部会按栈顶初始化）
  - stack_size 栈大小
  - priority 优先级（0 最高，值越大优先级越低）
  - tick 时间片长度（调度轮转基准）
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 任务创建

```c
yr_err_t yr_task_create( yr_task_t *task, yr_task_func_t entry, void *param, 
    void *stack_addr, yr_uint32_t stack_size, yr_uint8_t priority);
```

用于初始化一个任务。注意这个函数不会将任务加入调度队列之中。

作用与[任务初始化](#任务初始化)提到的API类似，两者唯一的区别是 `yr_task_create` 初始化的任务时间片长度固定为 `YR_DEFAULT_TIME_SLICE_TICKS`。

推荐使用 `yr_task_create` 而非 `yr_task_init` 来初始化函数，除非你真的有控制时间片长度的需求(这可能会破坏优先级原则)。

- 参数
  - 同[任务初始化](#任务初始化)的参数
- 返回值
  - 同[任务初始化](#任务初始化)的返回值

## 任务启动

```c
yr_err_t yr_task_start( yr_task_t *task);
```

将一个已经初始化但尚未参与调度的任务加入调度器管理。

- 适用状态
  - `YR_TASK_STATUS_INIT`
  - `YR_TASK_STATUS_SUSPENDED`
- 行为
  - 若任务处于 `INIT` 状态，会恢复其初始优先级和初始时间片
  - 任务会被置为 `READY` 并加入就绪队列
  - 若新启动任务的优先级高于当前运行任务，则可能立即触发一次调度
- 参数
  - task 任务对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 不可对未初始化的任务对象调用
  - 不建议在 ISR 中直接调用

## 任务删除

```c
yr_err_t yr_task_delete(yr_task_t *task);
```

删除一个任务。任务被删除后不会立刻彻底释放，而是先转为 `TERMINATED` 状态，随后由 idle 任务调用 `yr_task_cleanup_defunct()` 完成最终清理。

- 行为
  - 若任务当前在就绪队列中，会被移出调度器
  - 若任务当前在阻塞/挂起链表中，会被移出对应链表
  - 任务会从当前状态转换为 `TERMINATED`
  - 最终由 idle 任务回收并转为 `DELETED`
- 参数
  - task 要删除的任务对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 当前实现要求 `task` 不能为空
  - 不建议用于内核内部创建的任务，例如 idle 任务

## 任务挂起

```c
yr_err_t yr_task_suspend( yr_task_t *task);
```

将任务置为挂起状态，使其暂时不再参与调度。

- 适用状态
  - `YR_TASK_STATUS_READY`
  - `YR_TASK_STATUS_RUNNING`
  - `YR_TASK_STATUS_BLOCKED`
- 行为
  - 若任务处于 `READY/RUNNING`，会先从就绪队列移除，再置为 `SUSPENDED`
  - 若任务处于 `BLOCKED`，会停止其定时器并从阻塞链表中移除，再置为 `SUSPENDED`
  - 若挂起的是当前任务，则可能触发一次任务切换
- 参数
  - task 要挂起的任务对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 当前实现要求 `task` 不能为空
  - 不建议用于内核内部创建的任务，例如 idle 任务

## 相对延时

```c
void yr_task_sleep_ticks( yr_uint32_t ticks);
```

让当前任务相对延时指定的节拍数。

- 行为
  - 当前任务会从就绪队列中移出
  - 任务状态被置为 `BLOCKED`
  - 内部启动该任务的超时定时器，超时后自动恢复为 `READY`
  - 然后立即触发一次任务切换
- 参数
  - ticks 延时的节拍数
- 特殊情况
  - 当 `ticks == 0` 时，行为等价于主动让出 CPU 并触发一次调度
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

## 周期延时

```c
yr_err_t yr_task_sleep_until(yr_uint32_t *pre_ticks, yr_uint32_t inc_ticks);
```

按固定周期让当前任务延时，常用于周期任务。

- 参数
  - pre_ticks 记录上一次唤醒参考时刻的变量指针
  - inc_ticks 周期增量，单位为 tick
- 行为
  - 函数会先执行 `*pre_ticks += inc_ticks`
  - 若当前时间尚未到达目标时刻，则任务会阻塞到对应时间点
  - 若当前时间已经超过目标时刻，则立即返回，不再额外延时
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 典型用法

```c
yr_uint32_t pre_ticks = 0;

for(;;) {
    /* 周期任务内容 */
    yr_task_sleep_until(&pre_ticks, YR_MS_TO_TICKS(1000));
}
```

- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用
  - 第一次使用时通常应将 `pre_ticks` 初始化为 `0` 或当前 tick

## 清理僵尸任务

```c
void yr_task_cleanup_defunct(void);
```

清理所有处于 `TERMINATED` 状态的任务，并将其标记为 `DELETED`。

- 行为
  - 从 `yr_task_defunct_list` 中依次取出任务
  - 将任务状态改为 `DELETED`
  - 将其从 defunct 链表中移除
- 典型场景
  - 通常在 idle 任务中周期性调用
- 限制
  - 一般不需要由普通应用任务手动调用

## 控制当前任务状态/优先级

```c
yr_err_t yr_task_ctrl_current( yr_task_t *task, yr_uint32_t cmd, void *arg, yr_bool_t *need_switch);
```

用于获取任务当前状态、当前优先级，或者修改任务当前优先级。

- 参数
  - task 任务对象指针
  - cmd 控制命令，取值见 `yr_task_ctl_current_t`
  - arg 命令参数或输出参数
  - need_switch 若操作导致需要切换任务，则会被置为 `YR_TRUE`
- 支持的命令
  - `YR_TASK_CTL_GET_CUR_STATUS`：获取任务当前状态，结果写入 `arg`
  - `YR_TASK_CTL_GET_CUR_PRIORITY`：获取任务当前优先级，结果写入 `arg`
  - `YR_TASK_CTL_SET_CUR_PRIORITY`：将任务当前优先级设置为 `*(yr_uint8_t *)arg`
- 行为
  - 当修改的是 `READY/RUNNING` 任务时，会重新整理其在就绪队列中的位置
  - 当修改的是因 IPC 而阻塞的任务时，会根据 IPC 类型尝试重新整理其在阻塞队列中的顺序
  - 若优先级变化影响调度结果，则可能通过 `need_switch` 告知调用者需要切换任务
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 该函数既支持查询，也支持修改，不同命令下 `arg` 的含义不同，使用时需特别注意

## 设置任务初始优先级

```c
yr_err_t yr_task_set_priority( yr_task_t *task, yr_uint8_t priority);
```

设置任务的初始优先级。

- 行为
  - 会修改 `task->init_priority`
  - 若任务当前未因互斥锁优先级继承而临时提升优先级，则还会同步修改其当前优先级
  - 若优先级变化影响调度结果，则可能立即触发一次调度
- 参数
  - task 任务对象指针
  - priority 新的初始优先级
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 设置任务消息信息

```c
yr_err_t yr_task_set_msg( yr_task_t *task, void *source, void *msg, yr_uint8_t reason, yr_uint16_t notify);
```

设置任务的消息/阻塞信息。该接口通常由内核内部在任务阻塞、超时恢复、IPC 唤醒等场景下使用。

- 参数
  - task 任务对象指针
  - source 消息来源对象，例如定时器或 IPC 对象
  - msg 附加消息指针
  - reason 阻塞或消息原因，取值见 `yr_task_msg_reason_t`
  - notify 唤醒通知类型，取值见 `yr_task_msg_notify_t`
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 应用层一般不需要直接使用该函数

## 任务管理示例

```c
static yr_task_t led_task;
static yr_uint8_t led_task_stack[512];

static void led_task_entry(void *param)
{
    yr_uint32_t pre_ticks = 0;

    (void)param;

    for(;;) {
        led_toggle();
        yr_task_sleep_until(&pre_ticks, YR_MS_TO_TICKS(500));
    }
}

void app_task_init(void)
{
    yr_task_create(&led_task, led_task_entry, NULL,
        led_task_stack, sizeof(led_task_stack), 5);
    yr_task_start(&led_task);
}
```

