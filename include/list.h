#ifndef YUAN_RTOS_LIST_H
#define YUAN_RTOS_LIST_H

#include "yr_def.h"

#define YR_LIST_ENTRY(node, type, member) \
    YR_CONTAINER_OF(node, type, member)

/* 双向循环队列 
 * 链表可以用来模拟队列和栈：
 * 对于队列，可用 yr_list_insert_before 将数据插入到头节点之前，然后通过头节点向后遍历模拟
 * 对于栈，可以用 yr_list_insert_after 将数据插入到头节点之后，然后通过头节点向后遍历模拟
 */
typedef struct yr_list_t {
    struct yr_list_t* prev;
    struct yr_list_t* next;
} yr_list_t, yr_list_head_t;

/**
 * @brief 初始化链表节点或链表头。
 * @param list 要初始化的链表节点指针。
 */
void yr_list_init( yr_list_t* list);

/**
 * @brief 判断链表是否为空。
 * @param list 链表头指针。
 * @return 为空返回 YR_TRUE，否则返回 YR_FALSE。
 */
yr_bool_t yr_list_isempty( yr_list_t* list);

/**
 * @brief 在指定节点前插入一个节点。
 * @param list 目标位置节点指针。
 * @param node 待插入节点指针。
 */
void yr_list_insert_before( yr_list_t* list, yr_list_t* node);

/**
 * @brief 在指定节点后插入一个节点。
 * @param list 目标位置节点指针。
 * @param node 待插入节点指针。
 */
void yr_list_insert_after( yr_list_t* list, yr_list_t* node);

/**
 * @brief 删除指定节点前面的节点。
 * @param node 基准节点指针。
 */
void yr_list_delete_before( yr_list_t* node);

/**
 * @brief 删除指定节点后面的节点。
 * @param node 基准节点指针。
 */
void yr_list_delete_after( yr_list_t* node);

/**
 * @brief 将指定节点从链表中移除。
 * @param node 待移除节点指针。
 */
void yr_list_delete_self( yr_list_t* node);

#endif /* YUAN_RTOS_LIST_H */
