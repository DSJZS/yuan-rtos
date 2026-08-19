# 安装 Yuan RTOS

Yuan RTOS 以内核源码形式提供，没有复杂的安装步骤——把它编译进你的工程即可。

## 环境要求

- CMake ≥ 3.22（使用 CMake 集成时）
- 支持 C11 的编译器，推荐 `arm-none-eabi-gcc`（BSP 示例在 15.x 上验证）
- 或 IAR EWARM（BSP 提供 `rtos.ewp` 工程）

## 方式一：作为 CMake 子项目集成（推荐）

在工程 `CMakeLists.txt` 中加入：

```cmake
add_subdirectory(path/to/yuan-rtos)
target_link_libraries(your_target yuan::rtos)
```

`yuan::rtos` 已经配置好头文件路径（`include/` 与 `libcpu/CM3/`），直接在源码中 `#include "kernel.h"` 即可。

## 方式二：源码拷贝集成

不使用 CMake 时，将以下内容加入工程编译：

- `src/*.c`：内核源码
- `libcpu/CM3/port_gcc.c`、`libcpu/CM3/port_gcc.s`：移植层
- 头文件搜索路径：`include/`、`libcpu/CM3/`

## 方式三：使用 BSP 示例工程

仓库提供了 STM32F103C8T6 示例（`bsp/stm32f1c8`）：

```sh
cd bsp/stm32f1c8
cmake --preset Debug
cmake --build --preset Debug
```

也可以直接用 IAR 打开 `bsp/stm32f1c8/EWARM/rtos.ewp` 编译。

## 配置内核

编辑 `include/yr_config.h` 裁剪功能：

| 宏 | 说明 | 默认 |
| --- | --- | --- |
| `YR_TICK_RATE_HZ` | 系统 tick 频率 | 1000 |
| `YR_DEFAULT_TIME_SLICE_TICKS` | 默认时间片长度 | 10 |
| `YR_SUPPORT_DEBUG_LOG` | 日志输出 | 1 |
| `YR_PRINTF_BUF_SIZE` | 日志缓冲区 | 128 |
| `YR_SUPPORT_ASSERT` | 断言 | 1 |
| `YR_SUPPORT_PARAM_CHECK` | 参数检查 | 1 |
| `YR_SUPPORT_IPC` | IPC 总开关 | 1 |
| `YR_SUPPORT_SEMAPHORE` / `YR_SUPPORT_MUTEX` / `YR_SUPPORT_QUEUE` | 各 IPC 模块 | 1 |

## 最小验证

1. 实现 `yr_putc()`（或先关闭日志）
2. 配置 SysTick 中断频率与 `YR_TICK_RATE_HZ` 一致，并在中断中调用 `yr_tick_update()`
3. 参考 [快速上手](/tutorial/) 创建任务并启动内核

更详细的步骤见 [Yuan RTOS 教程](/tutorial/) 与 [移植指南](/tutorial/porting/)。
