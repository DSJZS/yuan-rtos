#ifndef YUAN_RTOS_TASK_H
#define YUAN_RTOS_TASK_H

#include "portable.h"

typedef struct yr_task_t {
    void* sp;
    void* entry;
    // void* exit;
    void* stack_addr;
    yr_uint32_t stack_size;
} yr_task_t;

void yr_task_init( yr_task_t* task, void* entry, void* stack_addr, yr_uint32_t stack_size);

#endif /* YUAN_RTOS_TASK_H */
