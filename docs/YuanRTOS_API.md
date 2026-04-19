![Yuan](images/YuanLogo.jpg)  
# Yuan RTOS API 文档

本文档涵盖当前版本的所有 API，介绍了涉及的函数原型、参数、返回值、行为、限制与典型用法。

## 相关约定

| 相 | 说明 |
| --- | --- |
| 基本类型 | 使用 `include/portable.h` 中定义的固件宽度基本类型 |
| 返回值 | 除了特定的移植接口，统一使用 `include/yr_def.h` 中定义的 yr_err_t、yr_bool_t |
| 优先级 | 范围为 `[0  YR_TASK_PRIORITY_MAX)`, 数值越小优先级越高（0 为最高） |
| 节拍(滴答) | 全局节拍，频率由 `YR_TICK_RATE_HZ` 决定 |
| 临界区 | 内核临界区由 `yr_irq_disable/enable` 保护；外部则利用 IPC 相关 API 进行保护 |
| 节拍定时回调 | 要求提供时基定时中断执行，如 `Systick` |
| 上下文 | 有些 API 无法同时用于任务或者ISR上下文，见下文说明 |

### 返回值定义

类型为 yr_err_t 的返回值 

| 返回值 | 含义 | 典型来源 |
| --- | --- | --- |
| YR_OK | 成功 | 正常执行 |
| YR_ERR | 失败 | 超时、为空、已满、未知错误 |
| YR_NULL | 空指针 | 提供了空指针作为参数 |
| YR_INVALID | 无效值 | 提供了一个无效对象作为参数 |

类型为 yr_bool_t 的返回值

| 返回值 | 含义 | 典型来源 |
| --- | --- | --- |
| YR_FALSE | 假 | 判断为假逻辑 |
| YR_TRUE | 真 | 判断为真逻辑 |

## 调试方法

### 日志打印

#### 使用方法

可通过 `include/yr_config.h` 中的 `YR_SUPPORT_DEBUG_LOG` 打开或者关闭。

用于通过串口等方式输出日志，供用户观察程序运行状况。

开启改功能后需要用户提供 `void yr_putc(char c)` 函数实现，这个函数用于打印单个字符，常见的实现如下：
```c
/* 平台为 STM32F103C8T6 ，通过串口打印*/
void yr_putc(char c)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}
```

由三个日志等级可供使用，分别如下：
| 日志等级 | 含义 |
| YR_DEBUG_ERROR | 错误日志 |
| YR_DEBUG_WARN | 警告日志 |
| YR_DEBUG_INFO | 消息日志 |

#### 日志打印示例

```c
/* 输出消息日志 "Hello World!" */
YR_DEBUG_LOG(YR_DEBUG_INFO, "Hello World!\r\n");
```

### 断点

#### 使用方法

可通过 `include/yr_config.h` 中的 `YR_SUPPORT_ASSERT` 打开或者关闭。

判断语句为假时会进入死循环并关闭中断，可以配合[日志打印](#日志打印)功能，输出错误信息(错误点所处的函数，行号，语句)。

#### 断点示例

```c
/* 表示当前任务指针不能为空 */
YR_ASSERT( current_task != NULL );
```

### 参数检查

#### 使用方法

可通过 `include/yr_config.h` 中的 `YR_SUPPORT_PARAM_CHECK` 打开或者关闭。

判断语句为真时会立刻返回指定的返回值，如果函数返回类型为 void，则返回 YR_RETURN_NONE 。

#### 参数检查示例

```c
/* 任务指针参数为空，返回 YR_NULL */
YR_PARAM_CHECK( task == NULL, YR_NULL );
```

## 数据结构

### 链表

Yuan RTOS 中的链表采用双向循环链表实现，链表头本身也是一个链表节点。

- 特点
  - 任意节点都知道前驱和后继，适合频繁插入和删除
  - 头节点不存放业务数据，主要用于标识一个链表
  - 空链表状态下，头节点的 `prev` 和 `next` 都指向自己
  - 可作为调度队列、阻塞队列、定时器链表等基础容器
- 适用场景
  - 任务就绪队列
  - IPC 阻塞队列
  - 软件定时器有序链表
  - 其它需要 O(1) 插入/删除的内核对象组织方式

#### 初始化链表

```c
void yr_list_init( yr_list_t* list);
```

用于初始化作为链表头的链表节点，其余链表节点可以不使用该函数进行初始化。

- 参数
  - list 链表节点指针

#### 判断链表是否为空

```c
yr_bool_t yr_list_isempty( yr_list_t* list);
```

用于判断链表是否为空。

- 参数
  - list 作为链表头的链表节点的指针(其余节点传入无意义)

#### 链表节点前插节点

```c
void yr_list_insert_before( yr_list_t* list, yr_list_t* node);
```

在一个指定的链表节点前插入链表节点。

- 参数
  - list 链表节点指针
  - node 要插入的链表节点的指针

#### 链表节点后插节点

```c
void yr_list_insert_after( yr_list_t* list, yr_list_t* node);
```

在一个指定的链表节点后插入链表节点。

- 参数
  - list 链表节点指针
  - node 要插入的链表节点的指针

#### 链表节点前删节点

```c
void yr_list_delete_before( yr_list_t* node);
```

删除一个指定的链表节点前的链表节点。

- 参数
  - node 链表节点指针

#### 链表节点后删节点

```c
void yr_list_delete_after( yr_list_t* node);
```

删除一个指定的链表节点后的链表节点。

- 参数
  - node 链表节点指针

#### 链表节点自删除

```c
void yr_list_delete_self( yr_list_t* node);
```

将一个节点从其锁在的链表删除。

- 参数
  - node 链表节点指针

#### 链表示例

```c
typedef struct demo_node_t {
    int value;
    yr_list_t list_node;
} demo_node_t;

static yr_list_head_t demo_list;
static demo_node_t node1 = { .value = 1 };
static demo_node_t node2 = { .value = 2 };

void demo_list_init(void)
{
    yr_list_init(&demo_list);

    yr_list_insert_before(&demo_list, &node1.list_node);
    yr_list_insert_before(&demo_list, &node2.list_node);
}
```

## 内核的初始化 & 启动

### 内核初始化

```c
void yr_kernel_init(void);
```

用于初始化调度器、定时器链表、idle 任务(空闲任务)。

- 必须在创建任务之前调用( 通常位于 main 函数，在硬件初始化之后调用)。

### 内核启动

```c
void yr_kernel_start(void);
```

用于启动首次调度，切换到优先级最高的任务。

- 执行该函数前用户必须至少创建一个任务( idle 任务为内核创建，不算在其中 )，否则会一直执行 idle 任务。
- 该函数执行后会默认会破坏 main 函数的调用链，无法再返回 main 函数，应该在完成所有初始化配置后调用。

## 移植接口

不建议应用层直接调用，一般通过其它 API 间接调用。

移植接口是 Yuan RTOS 与具体 CPU 架构、编译器和异常机制之间的边界层。

- 特点
  - 将内核通用逻辑与底层上下文切换细节分离
  - 负责中断开关、首任务启动、任务切换、初始栈构造等底层能力
  - 同一套内核代码可通过替换 `libcpu/<arch>/port_<compiler>.*` 适配不同平台
- 适用场景
  - 将 Yuan RTOS 移植到新的 MCU/CPU 架构
  - 更换工具链后重新实现底层切换逻辑
  - 分析任务切换、异常返回和栈布局等底层行为

### 关闭中断

```c
yr_uint32_t yr_irq_disable(void);
```

关闭常规中断，并返回关闭前的中断状态。

- 返回值
  - 返回先前的中断状态，一般用于后续恢复
- 行为
  - 进入内核临界区前由内核调用
  - 返回值应原样传给[恢复中断](#恢复中断)
- 说明
  - 这是一个移植层原语，不同架构实现方式可能不同

### 恢复中断

```c
void yr_irq_enable(yr_uint32_t disirq);
```

恢复先前保存的中断状态。

- 参数
  - disirq 由[关闭中断](#关闭中断)返回的状态值
- 行为
  - 将处理器中断屏蔽状态恢复到进入临界区之前
- 说明
  - 必须与 `yr_irq_disable()` 配对使用

### 初始化任务栈

```c
yr_uint8_t *yr_task_stack_init( void *entry, void *exit,  void *param, yr_uint8_t *stackaddr);
```

初始化一个任务第一次运行时所需的初始栈帧。

- 参数
  - entry 任务入口函数地址
  - exit 任务退出后跳转的函数地址
  - param 传递给任务入口函数的参数
  - stackaddr 任务栈顶地址
- 返回值
  - 返回初始化后的栈指针
- 行为
  - 构造与目标架构异常返回约定兼容的初始上下文
  - 保证任务被首次切入时可以像“正常运行中的线程”一样恢复执行
- 说明
  - 这是移植 RTOS 到新架构时最关键的接口之一

### 首任务切换

```c
void yr_task_first_switch_to( yr_uint32_t to);
```

启动调度器后，切换到第一个任务。

- 参数
  - to 第一个任务栈指针变量的地址
- 行为
  - 完成从启动环境到线程调度环境的首次过渡
- 说明
  - 一般只在调度器启动阶段调用一次

### 任务切换

```c
void yr_task_switch( yr_uint32_t from, yr_uint32_t to);
```

请求从当前任务切换到目标任务。

- 参数
  - from 当前任务栈指针变量地址
  - to 目标任务栈指针变量地址
- 行为
  - 记录旧任务与新任务的栈指针地址
  - 延后通过异常机制完成真正的上下文切换
- 说明
  - 调度器决定“切给谁”，移植层负责“怎么切过去”

### 查找最低位 1

```c
int yr_find_first_set(int value);
```

查找一个整数中最低位被置位的 bit 位置。

- 参数
  - value 输入整数
- 返回值
  - 返回最低位 1 的位置，若没有置位则返回 0
- 行为
  - 调度器通过该接口快速找到当前最高优先级的就绪任务
- 说明
  - 这也是就绪位图调度能保持高效率的关键接口之一

### 移植接口示例

以下是 Cortex-M3/GCC 端口中与任务切换相关的典型职责划分：

- `yr_irq_disable/yr_irq_enable`
  - 基于 `PRIMASK` 保存和恢复中断状态
- `yr_task_first_switch_to`
  - 设置 `PendSV` 为最低优先级，并通过 `SVC` 启动首任务
- `yr_task_switch`
  - 保存切换请求信息并触发 `PendSV`
- `PendSV_Handler`
  - 保存旧任务 `r4-r11`，恢复新任务 `r4-r11`，更新 `PSP`

## 任务管理

任务是 Yuan RTOS 中最核心的执行单元，调度器始终围绕任务对象 `yr_task_t` 进行管理。

- 特点
  - 使用静态任务对象和用户提供的栈空间，不依赖动态内存分配
  - 支持优先级抢占和同优先级时间片轮转
  - 支持延时、挂起、删除、优先级调整
  - 每个任务内置一个定时器对象，用于延时和 IPC 超时等待
  - 与 IPC、互斥锁优先级继承等机制紧密联动
- 典型任务状态
  - `INIT`：已初始化但尚未启动
  - `READY`：已进入就绪队列，等待运行
  - `RUNNING`：当前正在运行
  - `BLOCKED`：因延时或 IPC 等待而阻塞
  - `SUSPENDED`：被挂起，暂不参与调度
  - `TERMINATED/DELETED`：已退出并等待/完成清理
- 适用场景
  - 前后台并发业务拆分
  - 周期任务执行
  - 事件驱动任务
  - 配合 IPC 的生产者/消费者模型

### 任务初始化

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 任务创建

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

### 任务启动

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 不可对未初始化的任务对象调用
  - 不建议在 ISR 中直接调用

### 任务删除

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 当前实现要求 `task` 不能为空
  - 不建议用于内核内部创建的任务，例如 idle 任务

### 任务挂起

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 当前实现要求 `task` 不能为空
  - 不建议用于内核内部创建的任务，例如 idle 任务

### 相对延时

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

### 周期延时

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
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

### 清理僵尸任务

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

### 控制当前任务状态/优先级

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 该函数既支持查询，也支持修改，不同命令下 `arg` 的含义不同，使用时需特别注意

### 设置任务初始优先级

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 设置任务消息信息

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 应用层一般不需要直接使用该函数

### 任务管理示例

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

## 软件定时器

软件定时器由 `yr_timer_t` 表示，本质上是一个由系统 tick 驱动的超时对象。

- 特点
  - 定时器对象本身不创建独立任务
  - 所有定时器统一挂在内核定时器链表上管理
  - 超时回调在 `yr_tick_update()` 后续流程中执行，因此回调逻辑应尽量简短
- 适用场景
  - 任务延时
  - 超时等待
  - 周期性软件轮询

### 获取当前 tick

```c
yr_uint32_t yr_get_current_ticks(void);
```

获取当前系统 tick 计数值。

- 返回值
  - 当前 tick 值
- 说明
  - 返回值会随系统节拍持续递增
  - 该函数本身不是严格原子接口，若应用对一致性要求极高，可自行在临界区内读取

### 初始化定时器链表

```c
void yr_timer_list_init(void);
```

初始化内核的软件定时器链表。

- 行为
  - 将内部定时器链表恢复为空链表状态
- 说明
  - 该函数一般由 `yr_kernel_init()` 内部调用
  - 应用层通常不需要手动调用

### 定时器初始化

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - `timer` 与 `func` 不能为空
  - 建议定时器对象使用静态/全局存储，避免超时前对象失效

### 启动定时器

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 重新调用该函数可实现“重新装载/重启定时器”的效果

### 停止定时器

```c
yr_err_t yr_timer_stop( yr_timer_t *timer);
```

停止一个软件定时器。

- 参数
  - timer 定时器对象指针
- 行为
  - 将该定时器从内核定时器链表中移除
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 停止后不会触发超时回调，除非之后再次调用[启动定时器](#启动定时器)

### 设置定时器周期

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 若定时器已经在运行，通常应在设置后重新调用[启动定时器](#启动定时器)，使新周期立即生效

### 系统 tick 更新

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

### 默认超时回调

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

### 软件定时器示例

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

## 调度器

不建议应用层直接调用，一般通过其它 API 间接调用。

调度器负责维护系统中可运行任务的组织方式，并在合适时机决定当前应该运行哪个任务。

- 特点
  - 采用“优先级位图 + 每优先级链表”的组织方式
  - 高优先级任务优先运行，数值越小优先级越高
  - 同优先级任务通过时间片轮转和主动让出实现公平调度
  - 任务切换决策在通用层完成，真正上下文切换由移植层完成
- 适用场景
  - 管理所有 READY/RUNNING 任务
  - 响应任务创建、唤醒、阻塞、删除后的可运行集合变化
  - 为时间片调度和抢占式调度提供基础能力

### 获取当前任务

```c
yr_task_t* yr_sched_get_current(void);
```

获取当前正在运行的任务对象指针。

- 返回值
  - 当前运行任务指针
- 说明
  - 在任务上下文中可将其理解为“当前线程”
  - 在内核尚未启动前，返回值可能为 `NULL`

### 初始化调度器

```c
void yr_sched_init(void);
```

初始化调度器内部状态。

- 行为
  - 清空当前任务指针
  - 清空就绪优先级位图
  - 初始化每个优先级对应的就绪链表
  - 初始化僵尸任务链表
- 说明
  - 一般由 `yr_kernel_init()` 内部调用

### 启动调度器

```c
void yr_sched_start(void);
```

启动调度器并切换到第一个任务。

- 行为
  - 从就绪位图中找出当前最高优先级
  - 取出该优先级链表中的第一个任务作为首任务
  - 将其状态置为 `RUNNING`
  - 调用移植层接口切换到首任务
- 说明
  - 这是从“初始化阶段”进入“多任务运行阶段”的关键步骤

### 执行一次任务切换

```c
void yr_sched_switch(void);
```

根据当前就绪情况执行一次任务切换。

- 行为
  - 重新计算最高优先级就绪任务
  - 若目标任务与当前任务相同，则直接返回
  - 若目标任务不同，则更新任务状态并调用移植层执行切换
- 说明
  - 该函数既可由任务 API 间接触发，也可由超时唤醒等内核路径触发

### 插入就绪任务

```c
yr_err_t yr_sched_insert_task( yr_task_t* task);
```

将一个任务插入调度器的就绪队列。

- 参数
  - task 待插入任务对象指针
- 行为
  - 将任务插入到其当前优先级对应链表的尾部
  - 更新就绪优先级位图
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 任务被唤醒、启动或从阻塞态恢复时通常会走这条路径

### 移出就绪任务

```c
yr_err_t yr_sched_remove_task( yr_task_t* task);
```

将一个任务从调度器的就绪队列中移除。

- 参数
  - task 待移除任务对象指针
- 行为
  - 将任务从对应优先级链表中删除
  - 若该优先级已经没有其它就绪任务，则清除位图中的对应 bit
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 任务阻塞、挂起、删除时通常会先从调度器中移除

### 当前任务主动让出处理器

```c
void yr_sched_yield(void);
```

让当前任务主动让出 CPU。

- 行为
  - 将当前任务移动到同优先级就绪链表的后部
  - 若同优先级没有其它任务，则不会发生实际切换
  - 若存在同优先级其它任务，则随后触发一次调度
- 说明
  - 常用于同优先级任务之间主动轮转
  - `yr_task_sleep_ticks(0)` 的效果与“立即触发一次调度”接近

### 调度器示例

以下是调度器在常见运行路径中的典型工作方式：

- 任务启动
  - `yr_task_start()` 调用 `yr_sched_insert_task()` 将任务放入就绪队列
- 任务阻塞
  - `yr_task_sleep_ticks()` 或 IPC 等待会先调用 `yr_sched_remove_task()`
- 时间片耗尽
  - `yr_tick_update()` 中会调用 `yr_sched_yield()` 让同优先级任务轮转
- 高优先级任务被唤醒
  - 内核会在恢复该任务后进一步调用 `yr_sched_switch()`，使其尽快运行

## IPC

本项目所有的 IPC 功能对象，都要继承与 IPC 基类。

IPC 基类 `yr_ipc_base_t` 统一封装了阻塞任务链表和阻塞排队策略，信号量、互斥锁、队列都基于它构建。

- 排队策略
  - `YR_IPC_FLAG_FIFO`：按阻塞先后顺序排队
  - `YR_IPC_FLAG_PRIO`：按任务当前优先级排队，优先级越高越先被唤醒
- 说明
  - 应用一般不直接操作 `blocked_list`
  - 该基类更偏向内核内部基础设施，但公开 API 仍可用于扩展自定义 IPC 对象

### IPC 初始化

```c
yr_err_t yr_ipc_init( yr_ipc_base_t *ipc_base, yr_uint32_t flag);
```

初始化一个 IPC 基础对象。

- 参数
  - ipc_base IPC 基础对象指针
  - flag 阻塞任务排队策略，取值见 `yr_ipc_flag_t`
- 行为
  - 将对象标记为有效
  - 保存排队策略
  - 初始化阻塞任务链表
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 阻塞任务进入 IPC 等待队列

```c
yr_err_t yr_ipc_block_task( yr_ipc_base_t *ipc_base, yr_task_t *task);
```

将一个任务从调度器中移出，并加入某个 IPC 对象的阻塞队列。

- 参数
  - ipc_base IPC 基础对象指针
  - task 要阻塞的任务对象指针
- 行为
  - 任务从就绪队列移除
  - 任务状态被置为 `BLOCKED`
  - 任务按 FIFO 或优先级策略插入对应 IPC 阻塞队列
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 说明
  - 该接口通常由信号量、互斥锁、队列等上层对象内部调用

### 恢复 IPC 上等待的所有任务

```c
yr_err_t yr_ipc_resume_all( yr_ipc_base_t *ipc_base);
```

恢复某个 IPC 对象上等待的全部任务。

- 参数
  - ipc_base IPC 基础对象指针
- 行为
  - 依次取出阻塞队列中的所有任务
  - 停止这些任务的等待超时定时器
  - 将任务置为 `READY`
  - 任务唤醒通知被设置为 `YR_TASK_MN_WAIT_IPC_DELETED`
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 典型场景
  - 删除信号量
  - 删除互斥锁
  - 删除队列

### 重新整理 IPC 阻塞顺序

```c
yr_err_t yr_ipc_reorder_blocked_task( yr_ipc_base_t *ipc_base, yr_task_t *task);
```

在优先级阻塞策略下，重新整理某个已阻塞任务在 IPC 阻塞队列中的位置。

- 参数
  - ipc_base IPC 基础对象指针
  - task 需要重新排序的任务对象指针
- 行为
  - 仅当排队策略为 `YR_IPC_FLAG_PRIO` 时才有实际作用
  - 常用于任务优先级变化后保持阻塞队列顺序正确
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

## 信号量

本项目的信号量，资源为直接传递。

也就是说，若释放信号量时已经有任务在等待，那么资源会直接交给等待队列中的一个任务，而不是先简单增加计数再由别的任务竞争。

#### 信号量初始化

```c
yr_err_t yr_semaphore_init( yr_semaphore_t* sem, yr_uint16_t max_count,yr_uint16_t init_count, yr_uint32_t flag);
```

初始化一个计数型信号量。

- 参数
  - sem 信号量对象指针
  - max_count 信号量最大计数
  - init_count 信号量初始计数
  - flag 等待任务的排队策略
- 行为
  - 设置最大计数和当前计数
  - 初始化内部 IPC 基础对象
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - `max_count` 不能为 0
  - `init_count` 不能大于 `max_count`

#### 删除信号量

```c
yr_err_t yr_semaphore_delete( yr_semaphore_t* sem);
```

删除一个信号量对象。

- 行为
  - 唤醒所有阻塞在该信号量上的任务
  - 被唤醒任务会得到 `YR_TASK_MN_WAIT_IPC_DELETED` 通知
  - 该信号量被标记为无效
- 参数
  - sem 信号量对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

#### 获取信号量

```c
yr_err_t yr_semaphore_take( yr_semaphore_t* sem, yr_uint32_t wait_ticks);
```

在任务上下文中获取信号量。

- 参数
  - sem 信号量对象指针
  - wait_ticks 资源不可用时允许等待的 tick 数
- 行为
  - 若当前计数大于 0，则直接减 1 并返回成功
  - 若当前计数为 0 且 `wait_ticks == 0`，立即返回失败
  - 若当前计数为 0 且允许等待，则当前任务进入该信号量阻塞队列
  - 若等待超时，则返回失败
  - 若被正常唤醒，则返回成功
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

### 释放信号量

```c
yr_err_t yr_semaphore_give( yr_semaphore_t* sem);
```

在任务上下文中释放一个信号量。

- 参数
  - sem 信号量对象指针
- 行为
  - 若没有等待任务，则将计数加 1
  - 若已有任务等待，则直接恢复一个等待任务，不额外累加计数
  - 若被恢复任务优先级更高，则可能立即触发一次调度
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 失败场景
  - 当前计数已达到最大值，且没有等待任务可直接接收该资源

### 中断中获取信号量

```c
yr_err_t yr_semaphore_take_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);
```

在 ISR 中尝试获取信号量。

- 参数
  - sem 信号量对象指针
  - need_switch 当前实现中该参数不会被写入，可传 `NULL`
- 行为
  - 不允许阻塞等待
  - 若当前计数大于 0，则减 1 并返回成功
  - 若当前计数为 0，则立即返回失败
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 中断中释放信号量

```c
yr_err_t yr_semaphore_give_from_isr( yr_semaphore_t* sem, yr_bool_t *need_switch);
```

在 ISR 中释放一个信号量。

- 参数
  - sem 信号量对象指针
  - need_switch 若唤醒了更高优先级任务，则会被置为 `YR_TRUE`
- 行为
  - 不允许阻塞
  - 若已有任务等待，则直接恢复一个等待任务
  - 若没有任务等待，则增加计数
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 信号量示例

```c
static yr_semaphore_t sem;

void producer_task(void *param)
{
    for(;;) {
        produce_one_item();
        yr_semaphore_give(&sem);
    }
}

void consumer_task(void *param)
{
    for(;;) {
        if( yr_semaphore_take(&sem, YR_WAIT_FOREVER) == YR_OK ) {
            consume_one_item();
        }
    }
}
```

## 互斥锁

本项目使用的互斥锁本质上为递归锁，支持优先级继承，强制要求"谁持有，谁释放"，资源为直接传递。

### 互斥锁初始化

```c
yr_err_t yr_mutex_init( yr_mutex_t* mutex, yr_uint32_t flag);
```

初始化一个互斥锁对象。

- 参数
  - mutex 互斥锁对象指针
  - flag 等待任务的排队策略
- 行为
  - 将持有者置空
  - 将递归持有深度置 0
  - 初始化内部 IPC 基础对象
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 删除互斥锁

```c
yr_err_t yr_mutex_delete( yr_mutex_t* mutex);
```

删除一个互斥锁对象。

- 行为
  - 唤醒所有阻塞在该锁上的任务
  - 若该锁当前有持有者，则会同步修正持有者的 `hold_mutex_count`
  - 当持有者不再持有任何互斥锁时，会尝试恢复其初始优先级
  - 互斥锁最终被标记为无效
- 参数
  - mutex 互斥锁对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 获取互斥锁

```c
yr_err_t yr_mutex_take( yr_mutex_t* mutex, yr_uint32_t wait_ticks);
```

在任务上下文中获取互斥锁。

- 参数
  - mutex 互斥锁对象指针
  - wait_ticks 锁不可用时允许等待的 tick 数
- 行为
  - 若当前任务已经持有该锁，则递归深度加 1 并返回成功
  - 若当前锁无人持有，则当前任务成为 owner，递归深度置为 1
  - 若锁已被别的任务持有且不允许等待，则立即返回失败
  - 若锁已被别的任务持有且允许等待，则当前任务进入阻塞队列
  - 等待期间若当前任务优先级高于 owner，会触发优先级继承，临时提升 owner 的当前优先级
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用
  - 递归持有深度受 `YR_MUTEX_HOLD_MAX` 限制

### 释放互斥锁

```c
yr_err_t yr_mutex_give( yr_mutex_t* mutex);
```

在任务上下文中释放互斥锁。

- 参数
  - mutex 互斥锁对象指针
- 行为
  - 只有 owner 才能释放该锁，否则返回失败
  - 每次释放会先将递归深度减 1
  - 当递归深度减到 0 时，锁才真正交出
  - 若当前任务不再持有任何互斥锁，则会尝试恢复其初始优先级
  - 若此时有任务在等待，则资源会直接交给等待队列中的一个任务
  - 若新 owner 优先级更高，则可能立即触发一次调度
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 互斥锁示例

```c
static yr_mutex_t uart_mutex;

void log_task(void *param)
{
    for(;;) {
        if( yr_mutex_take(&uart_mutex, YR_WAIT_FOREVER) == YR_OK ) {
            uart_send_string("log message\r\n");
            yr_mutex_give(&uart_mutex);
        }
    }
}
```

## 队列

本项目的队列，资源不为直接传递，而是通过用户提供的缓冲区进行拷贝收发。

也就是说，发送方写入的是“数据副本”，接收方读取的是从环形缓冲区中取出的内容，而不是像信号量/互斥锁那样直接把资源控制权交给某个任务。

### 队列初始化

```c
yr_err_t yr_queue_init( yr_queue_t *queue, yr_uint32_t item_size, yr_uint8_t *buffer, yr_uint32_t buffer_size, yr_uint32_t flag);
```

初始化一个消息队列。

- 参数
  - queue 队列对象指针
  - item_size 单个元素大小
  - buffer 队列使用的外部缓冲区
  - buffer_size 缓冲区总字节数
  - flag 等待任务的排队策略
- 行为
  - 根据 `buffer_size / item_size` 计算队列容量
  - 初始化队列的读写游标和计数
  - 分别初始化发送等待队列和接收等待队列
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - `buffer_size` 必须是 `item_size` 的整数倍
  - 容量必须大于 0
  - 缓冲区生命周期必须覆盖整个队列使用期

### 删除队列

```c
yr_err_t yr_queue_delete( yr_queue_t *queue);
```

删除一个消息队列。

- 行为
  - 唤醒所有阻塞在发送和接收方向上的任务
  - 被唤醒任务会得到 `YR_TASK_MN_WAIT_IPC_DELETED` 通知
  - 队列内部状态被清空并标记为无效
- 参数
  - queue 队列对象指针
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 重置队列

```c
yr_err_t yr_queue_reset( yr_queue_t *queue);
```

清空队列中的现有内容。

- 参数
  - queue 队列对象指针
- 行为
  - 将 `head`、`tail`、`item_count` 恢复到初始状态
  - 不清除实际缓冲区中的历史字节内容，但这些内容会被视为无效
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 只有在没有任务阻塞等待发送或接收时才允许重置，否则返回失败

### 发送消息

```c
yr_err_t yr_queue_send( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);
```

在任务上下文中向队列发送一个元素。

- 参数
  - queue 队列对象指针
  - item 待发送元素的地址
  - wait_ticks 队列满时允许等待的 tick 数
- 行为
  - 若队列未满，则将 `item_size` 字节数据拷贝进环形缓冲区
  - 若有任务阻塞在接收方向，会在发送成功后恢复一个接收任务
  - 若队列已满且 `wait_ticks == 0`，立即返回失败
  - 若队列已满且允许等待，则当前任务阻塞直到有空位或等待超时
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

### 接收消息

```c
yr_err_t yr_queue_receive( yr_queue_t *queue, void *item, yr_uint32_t wait_ticks);
```

在任务上下文中从队列接收一个元素。

- 参数
  - queue 队列对象指针
  - item 接收缓冲区地址
  - wait_ticks 队列空时允许等待的 tick 数
- 行为
  - 若队列非空，则从环形缓冲区读出一个元素到 `item`
  - 若有任务阻塞在发送方向，会在接收成功后恢复一个发送任务
  - 若队列为空且 `wait_ticks == 0`，立即返回失败
  - 若队列为空且允许等待，则当前任务阻塞直到有数据或等待超时
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

### 中断中发送消息

```c
yr_err_t yr_queue_send_from_isr( yr_queue_t *queue, void *item, yr_bool_t *need_switch);
```

在 ISR 中向队列发送一个元素。

- 参数
  - queue 队列对象指针
  - item 待发送元素的地址
  - need_switch 若唤醒了更高优先级任务，则会被置为 `YR_TRUE`
- 行为
  - 不允许阻塞
  - 若队列已满则立即返回失败
  - 若发送成功且有任务等待接收，则会恢复一个接收任务
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 中断中接收消息

```c
yr_err_t yr_queue_receive_from_isr( yr_queue_t *queue, void *item, yr_bool_t *need_switch);
```

在 ISR 中从队列接收一个元素。

- 参数
  - queue 队列对象指针
  - item 接收缓冲区地址
  - need_switch 若唤醒了更高优先级任务，则会被置为 `YR_TRUE`
- 行为
  - 不允许阻塞
  - 若队列为空则立即返回失败
  - 若接收成功且有任务等待发送，则会恢复一个发送任务
- 返回值
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](#返回值定义)

### 队列示例

```c
typedef struct {
    int id;
    int value;
} sensor_msg_t;

static yr_queue_t sensor_queue;
static yr_uint8_t sensor_queue_buf[8 * sizeof(sensor_msg_t)];

void producer_task(void *param)
{
    sensor_msg_t msg;

    for(;;) {
        msg.id = 1;
        msg.value = read_sensor();
        yr_queue_send(&sensor_queue, &msg, YR_WAIT_FOREVER);
    }
}

void consumer_task(void *param)
{
    sensor_msg_t msg;

    for(;;) {
        if( yr_queue_receive(&sensor_queue, &msg, YR_WAIT_FOREVER) == YR_OK ) {
            handle_sensor_msg(&msg);
        }
    }
}
```

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

## 设计要点与局限

本节用于总结 Yuan RTOS 当前实现中的核心设计思路、优点与局限，帮助使用者理解“为什么这样设计”，以及“在哪些场景下需要额外注意”。

### 设计要点

- 调度器使用“优先级位图 + 每优先级双向循环链表”
  - 优点：可通过位图快速定位最高优先级就绪任务，典型路径下具备 O(1) 调度查找能力。
  - 优点：同优先级任务再通过链表组织，便于实现时间片轮转和主动让出。
- 通用内核与移植层分离
  - 优点：调度、任务、IPC、定时器等逻辑保持平台无关，方便后续移植到不同 Cortex-M 或其它架构。
  - 优点：上下文切换、中断控制等强平台相关逻辑集中在 `portable.h` 与 `libcpu/` 中，边界清晰。
- 任务对象和大多数内核对象由用户静态提供
  - 优点：不依赖动态内存分配，行为更可预期，更适合资源受限 MCU。
  - 优点：避免了动态分配失败、堆碎片等额外问题。
- 软件定时器与任务延时共用统一 tick 驱动框架
  - 优点：实现简单，超时语义统一，便于维护。
  - 优点：任务延时、IPC 超时、普通软件定时器都建立在同一套时间基上。
- IPC 阻塞队列支持 FIFO 与按优先级排队两种策略
  - 优点：应用可以根据公平性或实时性需求选择不同阻塞排序方式。
- 互斥锁支持递归持有与优先级继承
  - 优点：适合嵌套调用链中重复进入同一临界资源保护区。
  - 优点：在常见优先级反转场景下能提升系统实时性。

### 局限与注意事项

- 互斥锁的优先级继承属于惰性继承
  - 局限：只有在高优先级任务真正因获取锁失败而阻塞时，owner 才会被提升优先级。
  - 局限：当前实现没有维护“等待任务优先级集合”的精细恢复机制，因此在多锁或复杂嵌套场景下，任务当前优先级可能出现偏高持续一段时间的现象。
- 任务优先级位图当前基于 32 位整数
  - 局限：当前最大任务优先级数固定为 `YR_TASK_MAX_PRIORITY == 32`，继续扩展时需要同步调整位图实现。
- 软件定时器基于系统 tick 驱动
  - 局限：时间精度受 `YR_TICK_RATE_HZ` 限制，不适合高精度硬实时定时需求。
  - 局限：tick 越高，时基中断开销越大；tick 越低，延时粒度越粗。
- 软件定时器回调直接在 tick 更新相关路径中执行
  - 局限：回调逻辑不宜过重，否则会拉长中断后处理或调度相关路径时间。
- 队列采用“拷贝式收发”
  - 优点：数据所有权清晰，不要求发送方和接收方长期共享同一块对象。
  - 局限：当消息体较大或频繁传输时，拷贝开销会变得明显。
- 当前对象删除语义偏保守
  - 优点：任务删除先转 `TERMINATED` 再由 idle 回收，更安全，便于后续扩展到动态内存场景。
  - 局限：删除不是“立即彻底销毁”，文档和应用设计上需要接受这种延迟回收语义。
- 当前 API 对上下文要求较严格
  - 局限：任务态、ISR 态可调用函数并不完全重合，使用者需要明确区分普通版本与 `_from_isr` 版本。
- 当前内核强调“小而清晰”
  - 优点：实现容易理解、适合教学与裁剪。
  - 局限：暂未覆盖事件标志组、动态任务、SMP、更复杂的电源管理等高级特性。
