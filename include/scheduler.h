#ifndef YUAN_RTOS_SCHEDULER_H
#define YUAN_RTOS_SCHEDULER_H

#include "yr_def.h"
#include "task.h"

yr_task_t* yr_sched_get_current(void);
void yr_sched_init(void);
void yr_sched_start(void);
void yr_sched_switch(void);
yr_err_t yr_sched_insert_task( yr_task_t* task);
yr_err_t yr_sched_delete_task( yr_task_t* task);
void yr_sched_yield(void);

#endif /* YUAN_RTOS_SCHEDULER_H */
