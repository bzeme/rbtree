#pragma once
#include <stddef.h>

struct rb_node
{
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    int color;
#define RB_RED 0
#define RB_BLACK 1
};

struct rb_tree
{
    struct rb_node *root;
};

struct rb_ops;

typedef int (*rb_compare_fn)(void *l, void *r, void *context);
typedef int (*rb_compare_key_fn)(void *item, void *key, void *context);

struct rb_ops
{
    size_t offset;
    void *context;
    rb_compare_fn compare;
    rb_compare_key_fn compare_key;
};

/* color macros */
#define rb_is_red(node) ((node) && (node)->color == RB_RED)
#define rb_is_black(node) (!(node) || (node)->color == RB_BLACK)

#define rb_entry(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

extern void *rb_insert_item(void *item, struct rb_tree *tree, struct rb_ops *ops);
extern void *rb_search_item(void *key, struct rb_tree *tree, struct rb_ops *ops);

/* link the *node* to the *parent* at the *slot* */
extern void rb_link(struct rb_node *node, struct rb_node *parent, struct rb_node **slot, struct rb_tree *tree);
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new, struct rb_tree *tree);
extern void rb_remove(struct rb_node *node, struct rb_tree *tree);

extern struct rb_node *rb_leftmost(struct rb_node *node);
extern struct rb_node *rb_rightmost(struct rb_node *node);

extern struct rb_node *rb_next(struct rb_node *node);
extern struct rb_node *rb_prev(struct rb_node *node);

extern struct rb_node *rb_first(struct rb_tree *tree);
extern struct rb_node *rb_last(struct rb_tree *tree);
