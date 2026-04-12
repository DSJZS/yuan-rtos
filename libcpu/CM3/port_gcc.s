    .syntax unified
    .thumb
    .text

    .global yr_task_first_switch_to
    .global SVC_Handler
	
    .type yr_task_first_switch_to, %function
    .type SVC_Handler, %function

@ void yr_task_first_switch_to( yr_uint32_t to)
    .thumb_func
yr_task_first_switch_to:
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
