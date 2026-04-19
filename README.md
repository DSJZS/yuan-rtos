![Yuan](docs/images/YuanLogo.jpg)  
**English** | [中文](README_zh.md)
# Yuan RTOS Overview

Yuan RTOS is a lightweight real-time operating system mainly targeting Cortex-M MCUs. It is designed to provide priority-based and time-slice-based task scheduling, timers, and a basic IPC framework while keeping resource usage low.

## Features

Currently supported features include:

- Priority-based multitasking scheduler, where higher-priority tasks run first (smaller numeric value means higher priority)
- Time-slice round-robin scheduling among tasks at the same priority level
- Software timers, used to implement timeout handling as well as relative and periodic task delays
- O(1) ready-task insertion, removal, and priority lookup using a bitmap plus doubly circular linked lists
- Basic IPC mechanisms such as queues, semaphores, and recursive mutexes, with priority inheritance support for mutexes
- Portability to other platforms, with clear separation between generic kernel code and platform-specific porting code
- Lightweight logging support through a simplified built-in `printf` implementation to reduce logging overhead
- Feature trimming through configuration switches so unused functionality can be removed to reduce resource usage

## Directory Layout

| Directory | Purpose |
| --- | --- |
| `include/` | Public header files |
| `src/` | Generic kernel source files |
| `libcpu/` | Architecture/compiler-specific porting layer |
| `bsp/` | Board support package examples |
| `docs/` | Documents, manuals, images, and other resources |
| `LICENSE` | License file |

## Modules

The main modules are listed below:

| Module | Header | Description |
| --- | --- | --- |
| Kernel entry | `kernel.h` | Provides `yr_kernel_init()` and `yr_kernel_start()` |
| Task management | `task.h` | Task creation, start, deletion, suspend, and delay |
| Scheduler | `scheduler.h` | Current task query, context switch, and yield |
| Timer | `timer.h` | Tick update, software timers, and timeout wake-up |
| IPC base | `ipc.h` | Unified blocked-task queue management |
| Semaphore | `semaphore.h` | Counting semaphores and ISR APIs |
| Mutex | `mutex.h` | Recursive mutexes and priority inheritance |
| Queue | `queue.h` | Message queues and ISR APIs |
| List | `list.h` | Doubly circular linked-list utilities |
| Porting layer | `portable.h` | Interrupt control, context switching, stack initialization, etc. |
| Core definitions | `yr_def.h` | Common macros, error codes, logging macros, etc. |
| Kernel configuration | `yr_config.h` | Feature switches and default configuration |

## Configuration

Edit `include/yr_config.h` to configure feature trimming, `printf` buffer size, and other kernel options.

Common configuration items:

| Macro | Description | Default |
| --- | --- | --- |
| `YR_TICK_RATE_HZ` | System tick frequency | `1000` |
| `YR_DEFAULT_TIME_SLICE_TICKS` | Default time-slice length | `10` |
| `YR_SUPPORT_DEBUG_LOG` | Enable logging output | `1` |
| `YR_PRINTF_BUF_SIZE` | Log formatting buffer size | `128` |
| `YR_SUPPORT_ASSERT` | Enable assertions | `1` |
| `YR_SUPPORT_PARAM_CHECK` | Enable parameter checks | `1` |
| `YR_IDLE_TASK_STACK_SZIE` | Idle task stack size | `256` |
| `YR_SUPPORT_IPC` | Enable IPC infrastructure | `1` |
| `YR_SUPPORT_SEMAPHORE` | Enable semaphores | `1` |
| `YR_SUPPORT_MUTEX` | Enable mutexes | `1` |
| `YR_SUPPORT_QUEUE` | Enable message queues | `1` |

Notes:

- If `YR_SUPPORT_IPC` is disabled, semaphores, mutexes, and queues will all be unavailable
- If logging is enabled, it is recommended to override `yr_putc()` in the BSP

## Quick Start

Below is a minimal runnable example. For more details, see the documents and manuals under `docs/`.

Using a Cortex-M3 STM32F103C8T6 as an example, add the following code to `main.c`:

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
    /* Initialize clock, UART, SysTick, and other hardware first */
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

Then add the following to `SysTick_Handler()` or another system tick interrupt:

```c
/* The interrupt frequency must match YR_TICK_RATE_HZ */
void SysTick_Handler(void)
{
    /* Platform-specific code before this is omitted */
    yr_tick_update();
    /* Platform-specific code after this is omitted */
}
```

In `yr_config.h`, the following configuration is recommended to support UART logging:

```c
/* These are the default settings */
#define YR_SUPPORT_DEBUG_LOG            (1)
#define YR_PRINTF_BUF_SIZE              (128)
```

After programming and starting the MCU, the UART terminal should output something like:

```text
[INFO] Start
[INFO] task 0 : count = 0
[INFO] task 0 : count = 1
[INFO] task 0 : count = 2
[INFO] task 0 : count = 3
```

## Resource Usage

Test platform and conditions:

- Platform: `STM32F103C8T6`
- Compiler: `arm-none-eabi-gcc 15.2.1`
- Configuration: default configuration, no feature trimming
- Incremental measurement: compared against a baseline program without Yuan RTOS under the same optimization level and hardware environment

Under optimization level `-Os`:

| Scenario | RAM Increase | ROM Increase |
| --- | ---: | ---: |
| Kernel only | 640B | 2476B |
| [Quick Start](#quick-start) program | 1752B | 3216B |

Under optimization level `-O3`:

| Scenario | RAM Increase | ROM Increase |
| --- | ---: | ---: |
| Kernel only | 640B | 3976B |
| [Quick Start](#quick-start) program | 1752B | 4704B |

Under these test conditions, `-O3` mainly increases `ROM` usage compared with `-Os`, by about `1.46KB`, while `RAM` usage remains nearly unchanged.

For extremely resource-constrained environments, `-Os` is recommended. It is also recommended to disable parameter checking, assertions, and logging when possible. Avoid heavy formatted logging inside tasks if stack size is tight, since it can significantly increase the minimum required task stack.

## Porting

Porting Yuan RTOS to a new platform mainly requires two parts. For detailed methods, please read [`docs/YuanRTOS_API.md`](docs/YuanRTOS_API.md)。

### 1. Implement the porting-layer interfaces

First, implement the types and interfaces declared in `include/portable.h`, mainly related to:

- Interrupt disable/restore
- First-task startup
- Task switching
- Initial task stack-frame construction
- Finding the least significant set bit in an integer

The relevant files are recommended to be placed as:

```text
libcpu/<ISA>/port_<compiler>.c
libcpu/<ISA>/port_<compiler>.s
```

For example, for Cortex-M3 + GCC:

- `libcpu/CM3/port_gcc.c`
- `libcpu/CM3/port_gcc.s`

### 2. Provide a system tick source

You also need to configure a periodic interrupt that matches `YR_TICK_RATE_HZ` and call:

```c
yr_tick_update();
```

## License

```text
SPDX-License-Identifier: MIT
Copyright (c) 2026
```

## Contributing

Issues and PRs are welcome. When submitting, it is recommended to include:

- The observed problem and reproduction steps
- The target platform, compiler, and optimization level
- Whether `yr_config.h` was modified
- Logs, `map` files, or key screenshots if runtime issues are involved
