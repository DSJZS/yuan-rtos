#ifndef YUAN_RTOS_LIST_H
#define YUAN_RTOS_LIST_H

#include "yr_def.h"

#define YR_LIST_ENTRY(node, type, member) \
    YR_CONTAINER_OF(node, type, member)

/* 双向循环队列 */
typedef struct yr_list_t {
    struct yr_list_t* prev;
    struct yr_list_t* next;
} yr_list_t, yr_list_head_t;

/* 链表可以用来模拟队列和栈
 *
 * 对于队列，可用 yr_list_insert_before 将数据插入到头节点之前，然后通过头节点向后遍历模拟
 *
 * 对于栈，可以用 yr_list_insert_after 将数据插入到头节点之后，然后通过头节点向后遍历模拟
 */

/* 当 list 当作链表头节点初始化 */
void yr_list_init( yr_list_t* list);
/* 当 list 当作链表头节点，判断链表是否为空 */
yr_bool_t yr_list_isempty( yr_list_t* list);
/* 在 list 节点前插入节点，注意节点不要重复插入 */
void yr_list_insert_before( yr_list_t* list, yr_list_t* node);
/* 在 list 节点后插入节点，注意节点不要重复插入 */
void yr_list_insert_after( yr_list_t* list, yr_list_t* node);
/* 将 list 前面的节点从链表中删除，注意不要删除头节点 */
void yr_list_delete_before( yr_list_t* node);
/* 将 list 后面的节点从链表中删除，注意不要删除头节点 */
void yr_list_delete_after( yr_list_t* node);
/* 将 list 自身从链表中删除，注意不要删除头节点 */
void yr_list_delete_self( yr_list_t* node);

#endif /* YUAN_RTOS_LIST_H */
