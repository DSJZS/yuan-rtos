#include "list.h"

void yr_list_init( yr_list_t* list)
{
    list->prev = list;
    list->next = list;
}

yr_bool_t yr_list_isempty( yr_list_t* list)
{
    if( list->next == list )
        return YR_TRUE;
    else
        return YR_FALSE;
}

void yr_list_insert_before( yr_list_t* list, yr_list_t* node)
{
    node->prev = list->prev;
    node->next = list;
    list->prev->next = node;
    list->prev = node;
}

void yr_list_insert_after( yr_list_t* list, yr_list_t* node)
{
    node->prev = list;
    node->next = list->next;
    list->next->prev = node;
    list->next = node;
}

void yr_list_delete_before( yr_list_t* node)
{
    yr_list_delete_self( node->prev);
}

void yr_list_delete_after( yr_list_t* node)
{
    yr_list_delete_self( node->next);
}

void yr_list_delete_self( yr_list_t* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    yr_list_init(node);
}