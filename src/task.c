#include "task.h"

void yr_task_init( yr_task_t* task, void* entry, void* stack_addr, yr_uint32_t stack_size)
{
    task->stack_addr = stack_addr;
    task->stack_size = stack_size;

    task->sp = yr_task_stack_init( entry, (void*)0, (yr_uint8_t*)stack_addr + stack_size);
}
