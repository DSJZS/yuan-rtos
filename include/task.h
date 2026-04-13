#ifndef YUAN_RTOS_TASK_H
#define YUAN_RTOS_TASK_H

#include "portable.h"
#include "yr_def.h"

typedef struct yr_task_t {
    void *sp;
    void *entry;
    // void* exit;
    void *param;
    void *stack_addr;
    yr_uint32_t stack_size;
} yr_task_t;

typedef void (*yr_task_func_t)(void *param);

yr_err_t yr_task_init( yr_task_t *task, yr_task_func_t entry, void *param, void *stack_addr, yr_uint32_t stack_size);

#endif /* YUAN_RTOS_TASK_H */
