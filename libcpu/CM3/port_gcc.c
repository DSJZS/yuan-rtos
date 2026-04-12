#include "portable.h"

yr_uint32_t yr_prev_task_sp_p;
yr_uint32_t yr_next_task_sp_p;
yr_uint32_t yr_switch_flag;

typedef struct yr_context_t {
    yr_cpu_stack_t r4;
    yr_cpu_stack_t r5;
    yr_cpu_stack_t r6;
    yr_cpu_stack_t r7;
    yr_cpu_stack_t r8;
    yr_cpu_stack_t r9;
    yr_cpu_stack_t r10;
    yr_cpu_stack_t r11;

    /* Hardware-stacked on exception entry */
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
 * @brief Initialize thread stack frame for first run.
 * @param entry Thread entry function.
 * @param stackaddr Stack top (high address end).
 * @return New PSP value after frame placement.
 */
yr_uint8_t *yr_task_stack_init( void *entry, void *exit,  void *param, yr_uint8_t *stackaddr)
{
    yr_context_t *pstack;
    yr_uint8_t *psp;
    yr_uint8_t i;

    psp = stackaddr;

    /* 8-byte align per ARM procedure call & exception entry requirements. */
    /* aligned = value & ~(align - 1) */
    psp = (yr_uint8_t *)( ((yr_cpu_stack_t)psp) & ~((8) - 1) );  

     /* Reserve space for initial context frame */
    psp -= sizeof(yr_context_t);
    pstack = (yr_context_t *)psp;

    /* Clear frame area */
    for (i = 0; i < 16; i++)
        ((yr_cpu_stack_t *)pstack)[i] = 0;

    pstack->r0 = (yr_cpu_stack_t)param;
    pstack->psr = 0x01000000UL;             /* Default xPSR (Thumb bit set) */
    pstack->pc_r15  = (yr_cpu_stack_t)entry;    /* Entry point */
    pstack->lr_r14  = (yr_cpu_stack_t)exit;     /* If thread function returns */

    return psp;
}
