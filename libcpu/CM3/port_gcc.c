#include "portable.h"

typedef yr_uint32_t      yr_cpu_stack_t;

yr_uint32_t yr_prev_task_sp_p;
yr_uint32_t yr_next_task_sp_p;
yr_uint32_t yr_switch_flag;

typedef struct yr_context_t {
    /* 低位，由软件手动写入寄存器 */
    yr_cpu_stack_t r4;
    yr_cpu_stack_t r5;
    yr_cpu_stack_t r6;
    yr_cpu_stack_t r7;
    yr_cpu_stack_t r8;
    yr_cpu_stack_t r9;
    yr_cpu_stack_t r10;
    yr_cpu_stack_t r11;

    /* 高位，异常进入时由硬件自动压栈的寄存器 */
    yr_cpu_stack_t r0;
    yr_cpu_stack_t r1;
    yr_cpu_stack_t r2;
    yr_cpu_stack_t r3;
    yr_cpu_stack_t r12;
    yr_cpu_stack_t lr_r14;
    yr_cpu_stack_t pc_r15;
    yr_cpu_stack_t psr;
} yr_context_t;

/**
 * @brief 初始化任务首次运行所需的栈帧。
 * @param entry 任务入口函数。
 * @param exit 任务退出时跳转的函数。
 * @param param 传递给任务入口函数的参数。
 * @param stackaddr 栈顶地址。
 * @return 初始化后的 PSP 值。
 */
yr_uint8_t *yr_task_stack_init( void *entry, void *exit,  void *param, yr_uint8_t *stackaddr)
{
    yr_context_t *pstack;
    yr_uint8_t *psp;
    yr_uint8_t i;

    psp = stackaddr;

    /* 按 ARM 调用规范做 8 字节对齐 */
    psp = (yr_uint8_t *)( ((yr_cpu_stack_t)psp) & ~((8) - 1) );  

     /* 预留初始上下文栈帧空间 */
    psp -= sizeof(yr_context_t);
    pstack = (yr_context_t *)psp;

    /* 清空初始上下文内容 */
    for (i = 0; i < 16; i++)
        ((yr_cpu_stack_t *)pstack)[i] = 0;

    pstack->r0 = (yr_cpu_stack_t)param;         /* 根据约定，函数的第一个参数写入 R0 寄存器 */
    pstack->psr = 0x01000000UL;                 /* 默认 xPSR，确保 Thumb 位有效 */
    pstack->pc_r15  = (yr_cpu_stack_t)entry;    /* 任务首次运行入口 */
    pstack->lr_r14  = (yr_cpu_stack_t)exit;     /* 任务函数返回后的出口 */

    return psp;
}

/* 利用编译器内建指令查找最低位的 1 */
int yr_find_first_set(int value)
{
    return __builtin_ffs(value);
}
