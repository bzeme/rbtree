#include "list.h"

void list_head_init(struct list *list)
{
    list->prev = list;
    list->next = list;
}

void list_remove(struct list *item)
{
    struct list *prev = item->prev;
    struct list *next = item->next;

    prev->next = next;
    next->prev = prev;
}

void list_add(struct list *item, struct list *head)
{
    struct list *first = head->next;

    item->prev = head;
    item->next = first;

    first->prev = item;
    head->next = item;
}

void list_add_tail(struct list *item, struct list *head)
{
    struct list *tail = head->prev;

    item->prev = tail;
    item->next = head;

    tail->next = item;
    head->prev = item;
}

void list_replace(struct list *victim, struct list *item) {
    struct list *prev = item->prev;
    struct list *next = item->next;

    item->next = next;
    item->prev = prev;

    prev->next = item;
    next->prev = item;
}
