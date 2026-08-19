# IPC 基础

## IPC 初始化

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

## 阻塞任务进入 IPC 等待队列

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 说明
  - 该接口通常由信号量、互斥锁、队列等上层对象内部调用

## 恢复 IPC 上等待的所有任务

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)
- 典型场景
  - 删除信号量
  - 删除互斥锁
  - 删除队列

## 重新整理 IPC 阻塞顺序

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
  - 返回 YR_OK 表示成功，否则失败，见[返回值定义](/tutorial/api/#返回值定义)

