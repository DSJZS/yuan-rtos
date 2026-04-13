#ifndef YUAN_RTOS_DEF_H
#define YUAN_RTOS_DEF_H

typedef enum yr_err_t {
    YR_ERR_OK = 0,
    YR_ERR_PARAM,
} yr_err_t;

typedef enum yr_bool_t {
    YR_BOOL_FALSE = 0,
    YR_BOOL_TRUE,
} yr_bool_t;

/* 类似 linux 内核的 container_of 操作 */
#define YR_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))




#endif /* YUAN_RTOS_DEF_H */
