#include "scheduler.h"
#include "yr_config.h"
#include "portable.h"
#include "list.h"

yr_task_t* yr_current_task;
yr_uint32_t yr_thread_ready_priority_group;

/* 任务优先级表，为每个优先级任务都创建一个链表头
 * 这里相当于用链表来组成了 YR_TASK_MAX_PRIORITY 个队列
 * 这里规定向前插入后执行，向后插入先执行。
 */
yr_list_head_t yr_task_priority_table[YR_TASK_MAX_PRIORITY];
yr_list_head_t yr_task_defunct_list;

/* 对于任务而言，通过该函数总能得到自己的指针，可以认为是"原子地"读取。
 * 对于中断而言，这个函数是不原子的
 */
yr_task_t* yr_sched_get_current(void)
{
    return yr_current_task;
}

void yr_sched_init(void) 
{
    yr_uint8_t i = 0;

    yr_current_task = NULL;
    yr_thread_ready_priority_group = 0;
    for( i = 0 ; i < YR_TASK_MAX_PRIORITY ; ++i ) {
        yr_list_init( &yr_task_priority_table[i] );
    }
    yr_list_init( &yr_task_defunct_list);
}

/* 执行前必须执行过 yr_sched_init */
void yr_sched_start(void)
{
    yr_task_t *next_task;
    yr_uint32_t highest_ready_priority;

    highest_ready_priority = yr_find_first_set(yr_thread_ready_priority_group);
    YR_ASSERT( highest_ready_priority != 0 );
    highest_ready_priority -= 1;
    YR_ASSERT( highest_ready_priority < YR_TASK_MAX_PRIORITY );

    /* 这里假设链表头的下一个为新的任务 */
    /* 需要外接配合将旧任务放到链表末尾重新排队 */
    next_task = YR_LIST_ENTRY( yr_task_priority_table[highest_ready_priority].next, yr_task_t, list_node);
    
    yr_current_task = next_task;
    next_task->status = YR_TASK_STATUS_RUNNING;
    next_task->remaining_ticks = next_task->init_ticks;

    yr_task_first_switch_to( (yr_uint32_t)&next_task->sp);
}

/* 执行前必须执行过 yr_sched_start  */
void yr_sched_switch(void)
{
    yr_task_t *prev_task, *next_task;
    yr_uint32_t highest_ready_priority;
    yr_uint32_t disirq = 0;

    disirq = yr_irq_disable();

    highest_ready_priority = yr_find_first_set(yr_thread_ready_priority_group);
    YR_ASSERT( highest_ready_priority != 0 );
    highest_ready_priority -= 1;
    YR_ASSERT( highest_ready_priority < YR_TASK_MAX_PRIORITY );

    prev_task = yr_current_task;
    next_task = YR_LIST_ENTRY( yr_task_priority_table[highest_ready_priority].next, yr_task_t, list_node);

    if( prev_task == next_task ) {
        yr_irq_enable(disirq);
        return;
    }

    yr_current_task = next_task;
    if( prev_task && prev_task->status == YR_TASK_STATUS_RUNNING )
        prev_task->status = YR_TASK_STATUS_READY;

    next_task->status = YR_TASK_STATUS_RUNNING;
    yr_task_switch( (yr_uint32_t)&prev_task->sp, (yr_uint32_t)&next_task->sp);

    yr_irq_enable(disirq);
}

yr_err_t yr_sched_insert_task( yr_task_t* task)
{
    yr_uint32_t disirq = 0;

    YR_PARAM_CHECK(task == NULL, YR_NULL);

    disirq = yr_irq_disable();

    yr_list_insert_before( &yr_task_priority_table[task->current_priority], &task->list_node);

    yr_thread_ready_priority_group |= task->priority_mask;

    yr_irq_enable(disirq);

    return YR_OK;
}

yr_err_t yr_sched_remove_task( yr_task_t* task)
{
    yr_uint32_t disirq = 0;

    YR_PARAM_CHECK(task == NULL, YR_NULL);

    disirq = yr_irq_disable();

    yr_list_delete_self( &task->list_node);

    if( yr_list_isempty(&yr_task_priority_table[task->current_priority]) ) {
        yr_thread_ready_priority_group &= ~(task->priority_mask);
    }

    yr_irq_enable(disirq);

    return YR_OK;
}

/* 当前线程主动让出 CPU，把自己移到同优先级就绪队列后面，让同优先级的其他线程先运行 */
void yr_sched_yield(void)
{
    yr_uint32_t disirq = 0;
    yr_task_t *prev_task;
    yr_list_head_t *priority_list;

    disirq = yr_irq_disable();

    prev_task = yr_current_task;
    priority_list = &yr_task_priority_table[prev_task->current_priority];

    /* 同优先级没有下家 */
    if( priority_list->next == &(prev_task->list_node) &&
        priority_list->prev == &(prev_task->list_node)) {
        yr_irq_enable(disirq);
        return;
    }

    /* 将当前任务放到队列末尾 */
    yr_list_delete_self( &prev_task->list_node );
    yr_list_insert_before( priority_list, &prev_task->list_node);

    yr_irq_enable(disirq);

    yr_sched_switch();
}
