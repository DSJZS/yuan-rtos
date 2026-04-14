#include "timer.h"
#include "portable.h"
#include "task.h"
#include "scheduler.h"

volatile yr_uint32_t yr_tick;

void yr_tick_update(void)
{
    yr_uint32_t disirq = 0;
    yr_task_t * current_task;

    disirq = yr_irq_disable();

    current_task = yr_sched_get_current();
    if( current_task == NULL ) {
        yr_irq_enable(disirq);
        return;
    }

    --current_task->remaining_tick;
    if( current_task->remaining_tick == 0 ) {
        current_task->remaining_tick = current_task->init_tick;
        yr_irq_enable(disirq);
        yr_sched_yield();
    } 
    
    yr_irq_enable(disirq);
}