<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/public/yuan-rtos-logo-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/public/yuan-rtos-logo-light.svg">
    <img alt="Yuan RTOS" src="docs/public/yuan-rtos-logo-light.svg" width="640">
  </picture>
</p>

[English](README.md) | **中文** 
# Yuan RTOS 简介  
  
Yuan RTOS 是一个主要面向 Cortex-M 的轻量化实时操作系统，目标是在较少资源占用下提供基于优先级与时间片的多任务调度、定时器以及最基本的 IPC 框架。

## 功能

当前支持以下功能：
- 支持基于优先级的多任务调度，高优先级任务优先执行( 数值越小优先级越高 )
- 支持时间片轮转，同优先级任务之间依次执行
- 支持软件定时器，基于该功能实现了超时处理、任务的相对延时与周期延时
- 通过位图与双向循环链表支持调度器 O(1) 添加、删除、查找合适优先级任务
- 支持基本的 IPC 功能，如队列、信号量、(递归)互斥锁，其中互斥锁支持优先级继承
- 支持移植到其它平台，通用代码与平台相关代码分开管理并预留接口
- 支持轻量化打印功能，通过实现简易的 printf 函数，减少了打印功能的资源占用
- 支持通过配置文件裁减，删除不需要的功能减少资源占用

## 目录

| 目录 | 用处 |
| --- | --- |
| `include/` | 所有对外头文件 |
| `src/` | 内核通用源文件 |
| `libcpu/` | 与架构/编译器相关的移植层代码 |
| `bsp/` | 板级示例工程 |
| `docs/` | 文档、手册以及图片等资源 |
| `LICENSE` | 许可证文件 |

## 模块

当前主要模块如下：

| 模块 | 头文件 | 说明 |
| --- | --- | --- |
| 内核入口 | `kernel.h` | 提供 `yr_kernel_init()` 与 `yr_kernel_start()` |
| 任务管理 | `task.h` | 任务创建、启动、删除、挂起、延时 |
| 调度器 | `scheduler.h` | 当前任务获取、任务切换、让出 CPU |
| 定时器 | `timer.h` | tick 更新、软件定时器、超时唤醒 |
| IPC 基础层 | `ipc.h` | 统一阻塞队列管理 |
| 信号量 | `semaphore.h` | 计数信号量与 ISR 接口 |
| 互斥锁 | `mutex.h` | 递归互斥锁与优先级继承 |
| 队列 | `queue.h` | 消息队列与 ISR 接口 |
| 链表 | `list.h` | 双向循环链表工具 |
| 移植层 | `portable.h` | 中断控制、上下文切换、栈初始化等接口 |
| 基础定义 | `yr_def.h` | 公共宏、错误码、日志宏等 |
| 内核配置 | `yr_config.h` | 裁剪开关与默认配置 |

## 配置
编辑 `include/yr_config.h` 配置文件即可完成裁减、printf 缓冲区大小等配置。

常见配置项如下：

| 宏 | 说明 | 默认值 |
| --- | --- | --- |
| `YR_TICK_RATE_HZ` | 系统 tick 频率 | `1000` |
| `YR_DEFAULT_TIME_SLICE_TICKS` | 默认时间片长度 | `10` |
| `YR_SUPPORT_DEBUG_LOG` | 是否开启日志输出 | `1` |
| `YR_PRINTF_BUF_SIZE` | 日志格式化缓冲区大小 | `128` |
| `YR_SUPPORT_ASSERT` | 是否开启断言 | `1` |
| `YR_SUPPORT_PARAM_CHECK` | 是否开启参数检查 | `1` |
| `YR_IDLE_TASK_STACK_SZIE` | idle 任务栈大小 | `256` |
| `YR_SUPPORT_IPC` | 是否启用 IPC 基础设施 | `1` |
| `YR_SUPPORT_SEMAPHORE` | 是否启用信号量 | `1` |
| `YR_SUPPORT_MUTEX` | 是否启用互斥锁 | `1` |
| `YR_SUPPORT_QUEUE` | 是否启用消息队列 | `1` |

说明：

- 如果关闭 `YR_SUPPORT_IPC`，则信号量、互斥锁、队列等功能会一并失效
- 如果使用日志功能，建议在 BSP 中重写 `yr_putc()`

## 快速上手

下面给出一个最小可运行示例，更多说明请查看 `docs/` 下的文档、手册等资源。

以 Cortex-M3 的 STM32F103C8T6 为例，在 `main.c` 中添加如下代码：

```c
#include "kernel.h"
#include "task.h"
#include "yr_def.h"

yr_task_t task0;
yr_uint8_t task0_stack[1024];

void task_entry(void *param)
{
    int index = (int)param;
    int count = 0;
    yr_uint32_t pre_ticks = 0;

    for (;;) {
        YR_DEBUG_LOG(YR_DEBUG_INFO, "task %d : count = %d\r\n", index, count++);
        yr_task_sleep_until(&pre_ticks, YR_MS_TO_TICKS(1000));
    }
}

void yr_putc(char c)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

int main(void)
{
    /* 这里先完成时钟、串口、SysTick 等硬件初始化 */
    YR_DEBUG_LOG(YR_DEBUG_INFO, "Start\r\n");

    yr_kernel_init();

    yr_task_create(&task0, task_entry, (void *)0, task0_stack, sizeof(task0_stack), 0);
    yr_task_start(&task0);

    yr_kernel_start();

    while (1)
    {
    }
}
```

在 `SysTick_Handler()` 或其它系统时基中断中添加：

```c
/* 中断频率需要满足 YR_TICK_RATE_HZ 的配置 */
void SysTick_Handler(void)
{
    /* 前面的平台相关代码省略 */
    yr_tick_update();
    /* 后面的平台相关代码省略 */
}
```

在 `yr_config.h` 配置文件中，建议进行如下配置以支持串口日志打印：
```c
/* 这是默认的配置 */
#define YR_SUPPORT_DEBUG_LOG            (1)
#define YR_PRINTF_BUF_SIZE              (128) 
```

启动后，串口终端应输出类似如下内容：

```text
[INFO] Start
[INFO] task 0 : count = 0
[INFO] task 0 : count = 1
[INFO] task 0 : count = 2
[INFO] task 0 : count = 3
``` 

## 资源占用

测试平台与条件：

- 平台：`STM32F103C8T6`
- 编译器：`arm-none-eabi-gcc 15.2.1`
- 配置：默认配置，无功能裁剪
- 增量口径：相较于同优化等级、同硬件环境下“不使用 Yuan RTOS”的基线程序

优化等级 `-Os` 下：

| 场景 | RAM 增量 | ROM 增量 |
| --- | ---: | ---: |
| 仅内核 | 640B | 2476B |
| [快速上手](#快速上手)程序 | 1752B | 3216B |

优化等级 `-O3` 下：

| 场景 | RAM 增量 | ROM 增量 |
| --- | ---: | ---: |
| 仅内核 | 640B | 3976B |
| [快速上手](#快速上手)程序 | 1752B | 4704B |

在本次测试条件下，`-O3` 相较于 `-Os` 主要带来额外的 `ROM` 占用，增量越为 `1.46KB`，`RAM` 基本不变。

对于资源极为紧张的环境，建议使用 `-Os` 进行优化，并关闭参数检查、断言、日志打印等功能，避免在任务中使用完整的日志打印等功能，这会急剧地增大任务栈的最低大小要求。

## 移植

移植到新平台时，主要需要完成两部分工作。详细的方法请阅读 [`docs/tutorial/api/`](docs/tutorial/api/)。

### 1. 实现移植层接口

首先实现 `include/portable.h` 头文件中规定的接口与类型定义，主要与下面几个功能相关：

- 中断开关与状态恢复
- 首任务启动
- 任务切换
- 任务初始栈帧构造
- 查找整数最低位的置位位置

相关文件建议放在：

```text
libcpu/<ISA>/port_<compiler>.c
libcpu/<ISA>/port_<compiler>.s
```

例如 Cortex-M3 + GCC：

- `libcpu/CM3/port_gcc.c`
- `libcpu/CM3/port_gcc.s`

### 2. 提供系统 tick

还需要根据目标平台配置一个满足 `YR_TICK_RATE_HZ` 的周期中断，并在其中调用：

```c
yr_tick_update();
```

## 许可证

```text
SPDX-License-Identifier: MIT
Copyright (c) 2026
```

## 贡献

欢迎提交 Issue 和 PR。建议在提交时提供以下信息：

- 问题现象与复现步骤
- 使用的平台、编译器与优化等级
- 是否修改过 `yr_config.h`
- 若涉及运行异常，尽量附上日志、`map` 文件或关键截图
