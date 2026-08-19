# 信号量

## 释放信号量

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 失败场景
  - 当前计数已达到最大值，且没有等待任务可直接接收该资源

## 中断中获取信号量

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 中断中释放信号量

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 信号量示例

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

