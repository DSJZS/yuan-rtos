#ifndef YUAN_RTOS_DEF_H
#define YUAN_RTOS_DEF_H

/* 就绪 */
#define YR_TASK_STATUS_READY        0x01
/* 运行 */
#define YR_TASK_STATUS_RUNNING      0x02
/* 阻塞 */
#define YR_TASK_STATUS_BLOCKED      0x04
/* 挂起(暂停) */
#define YR_TASK_STATUS_SUSPENDED    0x08
/* 终止(待删除，僵尸任务) */
#define YR_TASK_STATUS_TERMINATED   0x10
/* 删除(不再被管理) */
#define YR_TASK_STATUS_DELETED      0x20

typedef enum yr_err_t {
    YR_OK = 0,
    YR_NULL,
    YR_INVALID,
} yr_err_t;

typedef enum yr_bool_t {
    YR_FALSE = 0,
    YR_TRUE,
} yr_bool_t;


/* 类似 linux 内核的 container_of 操作 */
#define YR_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))




#endif /* YUAN_RTOS_DEF_H */
