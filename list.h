#pragma once
#include <stddef.h>

struct list
{
    struct list *prev;
    struct list *next;
};

#define list_entry(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

void list_add(struct list *item, struct list *head);

void list_add_tail(struct list *item, struct list *head);

void list_remove(struct list *item);

void list_head_init(struct list *list);

void list_replace(struct list *victim, struct list *item);
