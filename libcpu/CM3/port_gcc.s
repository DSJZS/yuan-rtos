    .syntax unified
    .thumb
    .text

    .extern yr_prev_task_sp_p
    .extern yr_next_task_sp_p
    .extern yr_switch_flag

    .global yr_irq_disable
    .global yr_irq_enable
    .global yr_task_first_switch_to
    .global SVC_Handler
    .global yr_task_switch
    .global PendSV_Handler
	
    .type yr_irq_disable, %function
    .type yr_irq_enable, %function
    .type yr_task_first_switch_to, %function
    .type SVC_Handler, %function
    .type yr_task_switch, %function
    .type PendSV_Handler, %function

    .equ NVIC_INT_CTRL,  0xE000ED04
    .equ NVIC_PENDSVSET, 0x10000000
    .equ NVIC_SYSPRI2,   0xE000ED20
    .equ NVIC_PENDSV_PRI, 0x00FF0000

@ yr_uint32_t yr_irq_disable(void)
    .thumb_func
yr_irq_disable:    
    mrs     r0, PRIMASK          @ save current PRIMASK
    cpsid   i                    @ disable IRQ
    bx      lr

@ void yr_irq_enable(yr_uint32_t disirq)
    .thumb_func
yr_irq_enable:    
    msr     PRIMASK, r0          @ restore PRIMASK
    bx      lr

@ void yr_task_first_switch_to( yr_uint32_t to)
    .thumb_func
yr_task_first_switch_to:
    ldr r1, =NVIC_SYSPRI2
    ldr r2, [r1]
    ldr r3, =NVIC_PENDSV_PRI
    orr r2, r2, r3
    str r2, [r1]
    cpsie i
    svc #0

@ SVC exception starts the first task using the PSP value pointed to by r0.
    .thumb_func
SVC_Handler:
    ldr r1, [r0]              @ r1 = *(&task->sp) = task->sp
    ldmia r1!, {r4-r11}       @ restore software-saved regs
    msr psp, r1               @ PSP now points to hw-stacked frame

    ldr r0, =0xE000ED08
    ldr r0, [r0]              @ VTOR
    ldr r0, [r0]              @ initial MSP from vector table
    msr msp, r0

    ldr r0, =0xFFFFFFFD       @ return to Thread mode, use PSP
    bx  r0

@ void yr_task_switch( yr_uint32_t from, yr_uint32_t to)
    .thumb_func
yr_task_switch:
    mrs r12, PRIMASK
    cpsid i

    @ 读取 yr_switch_flag 
    @ 如果为 1 跳转到 task_reswitch
    @ 否则置 1 继续执行
    ldr r2, =yr_switch_flag
    ldr r3, [r2]

    cmp r3, #1
    beq task_reswitch
    mov r3, #1
    str r3, [r2]

    @ 修改 yr_prev_task_sp_p
    ldr r2, =yr_prev_task_sp_p
    str r0, [r2]

task_reswitch:
    @ 修改 yr_next_task_sp_p
    ldr r2, =yr_next_task_sp_p
    str r1, [r2]

    msr PRIMASK, r12
    
    @ 触发 PendSV
    ldr r2, =NVIC_INT_CTRL
    ldr r3, =NVIC_PENDSVSET
    str r3, [r2]
    
    bx lr

    .thumb_func
PendSV_Handler:
    @ 保存当前中断屏蔽状态
    mrs     r2, PRIMASK
    cpsid   i
 
    @ 如果 yr_switch_flag 为 0 直接退出，否则置 0
    ldr     r0, =yr_switch_flag
    ldr     r1, [r0]
    cbz     r1, pendsv_exit
    mov     r1, #0
    str     r1, [r0]

    ldr     r0, =yr_prev_task_sp_p
    ldr     r1, [r0]
    cbz     r1, switch_to_thread

    @ 将 r4-r11 压入旧任务的栈中，然后将(旧任务)新的栈顶指针保存到旧任务的 sp 中
    mrs     r1, psp
    stmdb   r1!, {r4-r11}
    ldr     r0, [r0]
    str     r1, [r0]

switch_to_thread:
    @ 将 r4-r11 从新任务的栈中取出，然后将(新任务)新的栈顶指针保存到 PSP
    ldr     r1, =yr_next_task_sp_p
    ldr     r1, [r1]
    ldr     r1, [r1]

    ldmia   r1!, {r4-r11}
    msr     psp, r1

pendsv_exit:
    @ 恢复中断屏蔽状态
    msr     PRIMASK, r2

    @ 把当前异常返回码 LR 改成“返回线程模式时使用 PSP”，然后执行异常返回
    orr     lr, lr, #0x04
    bx      lr
