# 队列

## 队列初始化

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - `buffer_size` 必须是 `item_size` 的整数倍
  - 容量必须大于 0
  - 缓冲区生命周期必须覆盖整个队列使用期

## 删除队列

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 重置队列

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 只有在没有任务阻塞等待发送或接收时才允许重置，否则返回失败

## 发送消息

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

## 接收消息

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用

## 中断中发送消息

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 中断中接收消息

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 队列示例

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

