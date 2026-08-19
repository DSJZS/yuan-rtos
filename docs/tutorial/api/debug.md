# 调试方法

## 日志打印

### 使用方法

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

### 日志打印示例

```c
/* 输出消息日志 "Hello World!" */
YR_DEBUG_LOG(YR_DEBUG_INFO, "Hello World!\r\n");
```

## 断点

### 使用方法

可通过 `include/yr_config.h` 中的 `YR_SUPPORT_ASSERT` 打开或者关闭。

判断语句为假时会进入死循环并关闭中断，可以配合[日志打印](#日志打印)功能，输出错误信息(错误点所处的函数，行号，语句)。

### 断点示例

```c
/* 表示当前任务指针不能为空 */
YR_ASSERT( current_task != NULL );
```

## 参数检查

### 使用方法

可通过 `include/yr_config.h` 中的 `YR_SUPPORT_PARAM_CHECK` 打开或者关闭。

判断语句为真时会立刻返回指定的返回值，如果函数返回类型为 void，则返回 YR_RETURN_NONE 。

### 参数检查示例

```c
/* 任务指针参数为空，返回 YR_NULL */
YR_PARAM_CHECK( task == NULL, YR_NULL );
```

