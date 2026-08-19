# BSP 中需要补充的内容

## 1. 时钟和基础硬件初始化

至少要保证：

- CPU 时钟已经稳定
- RAM 可正常使用
- 中断系统已可用

## 2. 系统 tick 源

通常使用：

- `SysTick`
- 通用定时器中断
- 平台提供的系统时基

## 3. 日志输出接口（可选）

如果开启了：

```c
#define YR_SUPPORT_DEBUG_LOG    (1)
```

则建议实现：

```c
void yr_putc(char c);
```

例如通过 UART 输出单字符。

## 4. 启动文件 / 向量表配置

如果你的移植实现依赖：

- `SVC_Handler`
- `PendSV_Handler`
- `SysTick_Handler`

那么必须保证这些异常入口已正确接入启动文件和向量表。

