# 互斥锁

## 互斥锁初始化

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 删除互斥锁

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 获取互斥锁

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 限制
  - 只能在任务上下文中调用，不可在 ISR 中调用
  - 递归持有深度受 `YR_MUTEX_HOLD_MAX` 限制

## 释放互斥锁

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 互斥锁示例

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

