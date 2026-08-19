# 函数速查表

## 任务可调用函数快速浏览表
下表用于快速查看“普通任务上下文”中通常可以直接调用的接口。

| 函数 | 说明 |
| --- | --- |
| `yr_get_current_ticks` | 获取当前系统 tick，适合做时间戳、超时统计 |
| `yr_task_sleep_ticks` | 当前任务相对延时，`ticks == 0` 时可用于主动让出 CPU |
| `yr_task_sleep_until` | 当前任务按固定周期延时，适合周期任务 |
| `yr_task_create` / `yr_task_init` | 初始化任务对象，通常更适合在系统初始化阶段调用 |
| `yr_task_delete` | 删除指定任务，不建议删除内核内部任务 |
| `yr_task_suspend` | 挂起指定任务 |
| `yr_task_start` | 启动已初始化或已挂起的任务 |
| `yr_task_set_priority` | 修改任务初始优先级 |
| `yr_semaphore_init` / `yr_semaphore_delete` | 初始化或删除信号量对象 |
| `yr_semaphore_take` | 任务上下文获取信号量，支持阻塞等待 |
| `yr_semaphore_give` | 任务上下文释放信号量 |
| `yr_mutex_init` / `yr_mutex_delete` | 初始化或删除互斥锁对象 |
| `yr_mutex_take` | 任务上下文获取互斥锁，支持阻塞等待与优先级继承 |
| `yr_mutex_give` | 任务上下文释放互斥锁 |
| `yr_queue_init` / `yr_queue_delete` | 初始化或删除消息队列对象 |
| `yr_queue_reset` | 重置队列内容，要求当前没有任务阻塞在该队列上 |
| `yr_queue_send` | 任务上下文发送队列消息，支持阻塞等待 |
| `yr_queue_receive` | 任务上下文接收队列消息，支持阻塞等待 |
| `yr_timer_init` | 初始化软件定时器对象 |
| `yr_timer_start` | 启动软件定时器 |
| `yr_timer_stop` | 停止软件定时器 |
| `yr_timer_set_ticks` | 修改软件定时器周期 |
| `YR_DEBUG_LOG` / `yr_printf` | 打印日志，适合调试使用 |

补充说明：

- 以上表格主要面向“普通应用任务”，并非表示这些函数在任意阶段都适合调用。
- `yr_kernel_init()` 和 `yr_kernel_start()` 一般在 `main()` 初始化阶段调用，而不是在任务函数中调用。
- `yr_task_create()`、`yr_task_init()`、各类 `*_init()` 与 `*_delete()` 接口通常也可以在任务上下文中调用，但更常见的是在系统初始化阶段统一完成对象创建。
- `yr_timer_list_init()`、`yr_sched_init()`、`yr_sched_start()`、`yr_ipc_block_task()`、`yr_ipc_resume_all()`、`yr_ipc_reorder_blocked_task()`、`yr_task_set_msg()` 等接口更偏内核内部使用，因此没有放进应用任务快速浏览表。
- `scheduler.h` 与 `portable.h` 中的大部分接口属于内核/移植层接口，虽然有些在技术上可调用，但不建议应用任务直接使用。
- 可能导致阻塞的函数包括 `yr_semaphore_take()`、`yr_mutex_take()`、`yr_queue_send()`、`yr_queue_receive()`、`yr_task_sleep_ticks()`、`yr_task_sleep_until()`，调用前要确认当前任务允许被阻塞。

## ISR 可调用函数快速浏览表
下表用于快速查看“中断服务函数 ISR 上下文”中允许直接调用的接口。

| 函数 | 说明 |
| --- | --- |
| `yr_tick_update` | 系统节拍更新函数，通常在 `SysTick_Handler()` 中调用 |
| `yr_semaphore_take_from_isr` | ISR 中尝试获取信号量，不允许阻塞 |
| `yr_semaphore_give_from_isr` | ISR 中释放信号量，不允许阻塞 |
| `yr_queue_send_from_isr` | ISR 中向队列发送消息，不允许阻塞 |
| `yr_queue_receive_from_isr` | ISR 中从队列接收消息，不允许阻塞 |
| `yr_sched_switch` | 当 ISR 中 `need_switch == YR_TRUE` 时，可调用它请求一次任务切换 |
| `yr_get_current_ticks` | 读取当前 tick 值，适合做简单时间判断 |
| `YR_DEBUG_LOG` / `yr_printf` | 技术上可调用，但通常不建议在高频 ISR 中大量使用 |

补充说明：

- ISR 中原则上只能调用“不会阻塞、不会主动切换上下文、明确提供 ISR 版本”的接口。
- 不能在 ISR 中调用 `yr_task_sleep_ticks()`、`yr_task_sleep_until()`、`yr_semaphore_take()`、`yr_mutex_take()`、`yr_queue_send()`、`yr_queue_receive()` 等可能阻塞当前执行流的函数。
- 当前 RTOS 为 ISR 版本 IPC 接口提供了 `need_switch` 输出参数，表示“是否有更高优先级任务已就绪”。当 `need_switch == YR_TRUE` 时，通常应在 ISR 末尾调用 `yr_sched_switch()` 请求调度，以便在退出中断后尽快切换到目标任务。
- 这里调用 `yr_sched_switch()` 的含义更接近“发起一次切换请求”，真正的上下文切换仍由底层异常返回路径和移植层配合完成。
- 互斥锁没有提供 ISR 版本接口，因此 `yr_mutex_take()` / `yr_mutex_give()` 都不应在中断中调用。
- 除了上述这种 `need_switch == YR_TRUE` 后调用 `yr_sched_switch()` 的典型用法外，其它大多数 `yr_sched_*()`、`yr_kernel_init()`、`yr_kernel_start()`、`yr_task_switch()` 等初始化、调度与移植层接口都不应在 ISR 中直接调用。

## API 调用场景矩阵
下表用于更完整地查看各公开接口更适合在哪类场景中使用。

说明：

- `推荐`：该场景下是典型用法。
- `可用`：技术上可以使用，但通常不是首选场景。
- `谨慎`：只有明确理解实现语义时才建议使用。
- `内部`：更偏内核/移植层内部使用，不建议应用直接调用。
- `否`：不应在该场景中调用。

| 函数 | 初始化阶段 | 任务上下文 | ISR | 内核内部更常用 |
| --- | --- | --- | --- | --- |
| `yr_kernel_init` | 推荐 | 否 | 否 | 否 |
| `yr_kernel_start` | 推荐 | 否 | 否 | 否 |
| `yr_list_init` | 推荐 | 可用 | 否 | 推荐 |
| `yr_list_isempty` | 可用 | 可用 | 否 | 推荐 |
| `yr_list_insert_before` | 可用 | 可用 | 否 | 推荐 |
| `yr_list_insert_after` | 可用 | 可用 | 否 | 推荐 |
| `yr_list_delete_before` | 可用 | 可用 | 否 | 推荐 |
| `yr_list_delete_after` | 可用 | 可用 | 否 | 推荐 |
| `yr_list_delete_self` | 可用 | 可用 | 否 | 推荐 |
| `yr_task_init` | 推荐 | 可用 | 否 | 否 |
| `yr_task_create` | 推荐 | 可用 | 否 | 否 |
| `yr_task_start` | 推荐 | 可用 | 否 | 否 |
| `yr_task_delete` | 否 | 推荐 | 否 | 否 |
| `yr_task_suspend` | 否 | 推荐 | 否 | 否 |
| `yr_task_sleep_ticks` | 否 | 推荐 | 否 | 否 |
| `yr_task_sleep_until` | 否 | 推荐 | 否 | 否 |
| `yr_task_cleanup_defunct` | 否 | 谨慎 | 否 | 推荐 |
| `yr_task_ctrl_current` | 否 | 谨慎 | 否 | 推荐 |
| `yr_task_set_priority` | 否 | 推荐 | 否 | 否 |
| `yr_task_set_msg` | 否 | 否 | 否 | 推荐 |
| `yr_get_current_ticks` | 可用 | 推荐 | 可用 | 推荐 |
| `yr_timer_list_init` | 推荐 | 否 | 否 | 推荐 |
| `yr_timer_init` | 推荐 | 可用 | 否 | 否 |
| `yr_timer_start` | 可用 | 推荐 | 否 | 推荐 |
| `yr_timer_stop` | 可用 | 推荐 | 否 | 推荐 |
| `yr_timer_set_ticks` | 可用 | 推荐 | 否 | 推荐 |
| `yr_tick_update` | 否 | 否 | 推荐 | 否 |
| `yr_timeout_default_func` | 否 | 否 | 否 | 推荐 |
| `yr_sched_get_current` | 否 | 谨慎 | 谨慎 | 推荐 |
| `yr_sched_init` | 推荐 | 否 | 否 | 推荐 |
| `yr_sched_start` | 推荐 | 否 | 否 | 推荐 |
| `yr_sched_switch` | 否 | 谨慎 | 谨慎 | 推荐 |
| `yr_sched_insert_task` | 否 | 否 | 否 | 推荐 |
| `yr_sched_remove_task` | 否 | 否 | 否 | 推荐 |
| `yr_sched_yield` | 否 | 谨慎 | 否 | 推荐 |
| `yr_ipc_init` | 推荐 | 可用 | 否 | 推荐 |
| `yr_ipc_block_task` | 否 | 否 | 否 | 推荐 |
| `yr_ipc_resume_all` | 否 | 否 | 否 | 推荐 |
| `yr_ipc_reorder_blocked_task` | 否 | 否 | 否 | 推荐 |
| `yr_semaphore_init` | 推荐 | 可用 | 否 | 否 |
| `yr_semaphore_delete` | 可用 | 推荐 | 否 | 否 |
| `yr_semaphore_take` | 否 | 推荐 | 否 | 否 |
| `yr_semaphore_give` | 否 | 推荐 | 否 | 否 |
| `yr_semaphore_take_from_isr` | 否 | 否 | 推荐 | 否 |
| `yr_semaphore_give_from_isr` | 否 | 否 | 推荐 | 否 |
| `yr_mutex_init` | 推荐 | 可用 | 否 | 否 |
| `yr_mutex_delete` | 可用 | 推荐 | 否 | 否 |
| `yr_mutex_take` | 否 | 推荐 | 否 | 否 |
| `yr_mutex_give` | 否 | 推荐 | 否 | 否 |
| `yr_queue_init` | 推荐 | 可用 | 否 | 否 |
| `yr_queue_delete` | 可用 | 推荐 | 否 | 否 |
| `yr_queue_reset` | 可用 | 推荐 | 否 | 否 |
| `yr_queue_send` | 否 | 推荐 | 否 | 否 |
| `yr_queue_receive` | 否 | 推荐 | 否 | 否 |
| `yr_queue_send_from_isr` | 否 | 否 | 推荐 | 否 |
| `yr_queue_receive_from_isr` | 否 | 否 | 推荐 | 否 |
| `yr_irq_disable` | 否 | 谨慎 | 谨慎 | 推荐 |
| `yr_irq_enable` | 否 | 谨慎 | 谨慎 | 推荐 |
| `yr_task_stack_init` | 否 | 否 | 否 | 推荐 |
| `yr_task_first_switch_to` | 否 | 否 | 否 | 推荐 |
| `yr_task_switch` | 否 | 否 | 否 | 推荐 |
| `yr_find_first_set` | 否 | 否 | 否 | 推荐 |
| `yr_printf` / `YR_DEBUG_LOG` | 可用 | 推荐 | 谨慎 | 可用 |

补充说明：

- `yr_sched_switch()` 在任务和 ISR 两列标为 `谨慎`，是因为它通常只应在内核路径中，或在 ISR 中 `need_switch == YR_TRUE` 时使用。
- `yr_sched_get_current()` 在任务和 ISR 中都能读到当前任务指针，但它更偏内核辅助接口，应用层一般不必依赖。
- 链表接口属于通用数据结构工具，若应用自己管理静态链表可在初始化或任务中使用；但若链表节点由内核对象占用，就不应越过内核直接操作。
- `yr_irq_disable()` / `yr_irq_enable()` 对应用层并非绝对不可用，但它们属于非常底层的临界区原语，若误用会直接影响系统实时性与中断响应。

