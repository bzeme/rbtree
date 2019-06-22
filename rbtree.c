#include "rbtree.h"
#include <stddef.h>
#define NDEBUG
#include <assert.h>

/**
 * RB-tree:
 *  1. Each node is either red or black.
 *  2. The root is black.
 *  3. All leaves (NIL) are black.
 *  4. If a node is red, then both its children are black.
 *  5. Every path from a given node to any of its descendant NIL nodes contains the same number of black nodes.
 * 
 * Reference:
 *   -  http://en.wikipedia.org/wiki/Rbtree
 *   -  https://www.geeksforgeeks.org/red-black-tree-set-1-introduction-2/
 * 
 */

void *rb_insert_item(void *item, struct rb_tree *tree, struct rb_ops *ops)
{
    rb_compare_fn compare = ops->compare;
    size_t offset = ops->offset;
    void *cmp_ctx = ops->context;

    struct rb_node *parent = NULL;
    struct rb_node **slot = &tree->root;

    /* serch for the slot */
    while (*slot)
    {
        void *tmpi = (void *)*slot - offset;
        int result = compare(tmpi, item, cmp_ctx);

        parent = *slot;

        if (result > 0) {
            slot = &parent->left;
        } else if (result < 0) {
            slot = &parent->right;
        } else {
            /* found the identical key */
            return tmpi;
        }
    }

    struct rb_node * node = item + offset;
    rb_link(node, parent, slot, tree);

    return item;
}

void *rb_search_item(void *key, struct rb_tree *tree, struct rb_ops *ops)
{
    struct rb_node * node = tree->root;
    size_t offset = ops->offset;
    void *cmp_ctx = ops->context;
    rb_compare_key_fn compare = ops->compare_key;
    
    while (node) {
        void *item = node - offset;
        int result = compare(item, key, cmp_ctx);

        if (result > 0) {
            node = node->left;
        } else if (result < 0) {
            node = node->right;
        } else {
            return item;
        }
    }

    return NULL;
}

#define rb_set_red(node) ((node)->color = RB_RED)
#define rb_set_black(node) ((node)->color = RB_BLACK)

static inline void _rb_rotate_left(struct rb_node *node, struct rb_tree *tree)
{
    struct rb_node *parent = node->parent;
    struct rb_node *new = node->right;
    struct rb_node *child = new->left;

    assert(new);
    assert(new->parent == node);

    /* parent <--> new */
    if (parent)
    {
        if (parent->left == node)
        {
            parent->left = new;
        }
        else
        {
            parent->right = new;
        }
    }
    else
    {
        tree->root = new;
    }
    new->parent = parent;

    /* node <--> new */
    node->parent = new;
    new->left = node;

    /* node <--> child */
    if (child)
    {
        child->parent = node;
    }
    node->right = child;
}

static inline void _rb_rotate_right(struct rb_node *node, struct rb_tree *tree)
{
    struct rb_node *parent = node->parent;
    struct rb_node *new = node->left;
    struct rb_node *child = new->right;

    assert(new);
    assert(new->parent == node);

    /* parent <--> new */
    if (parent)
    {
        if (parent->left == node)
        {
            parent->left = new;
        }
        else
        {
            parent->right = new;
        }
    }
    else
    {
        tree->root = new;
    }
    new->parent = parent;

    /* node <--> new */
    node->parent = new;
    new->right = node;

    /* node <--> child */
    if (child)
    {
        child->parent = node;
    }
    node->left = child;
}

/* rebalance the tree */
static inline void _rb_link_color(struct rb_node *node, struct rb_tree *tree)
{
    struct rb_node *parent, *gparent, *uncle;

    assert(rb_is_red(node));

    while ((parent = node->parent) && rb_is_red(parent))
    {
        assert(parent->left == node || parent->right == node);
        gparent = parent->parent;

        if (gparent->left == parent)
        {
            uncle = gparent->right;

            if (rb_is_red(uncle))
            {
                /* case 3 */
                rb_set_black(parent);
                rb_set_black(uncle);
                rb_set_red(gparent);
                node = gparent;
                continue;
            }
            else
            {
                /* case 4 */
                /* step 1 */
                if (parent->right == node)
                {
                    _rb_rotate_left(parent, tree);
                    node = parent;
                    parent = node->parent;
                } else {
                    assert(parent->left == node);
                }

                /* step 2 */
                _rb_rotate_right(gparent, tree);
                rb_set_red(gparent);
                rb_set_black(parent);
                break;
            }
        }
        else
        {
            assert(gparent->right == parent);
            uncle = gparent->left;

            if (rb_is_red(uncle))
            {
                /* case 3 */
                rb_set_black(parent);
                rb_set_black(uncle);
                rb_set_red(gparent);
                node = gparent;
                continue;
            }
            else
            {
                /* case 4 */
                /* step 1 */
                if (parent->left == node)
                {
                    _rb_rotate_right(parent, tree);
                    node = parent;
                    parent = node->parent;
                } else {
                    assert(parent->right == node);
                }

                /* step 2 */
                _rb_rotate_left(gparent, tree);
                rb_set_red(gparent);
                rb_set_black(parent);
                break;
            }
        }
    }

    /* 
     * case 1 - node is root
     * case 2 - node's parent is black
     */
    /* make sure that the root is black */
    rb_set_black(tree->root);
}

void rb_link(struct rb_node *node, struct rb_node *parent, struct rb_node **slot, struct rb_tree *tree)
{
    node->left = node->right = NULL;
    node->parent = parent;
    *slot = node;

    assert(!parent || parent->left == node || parent->right == node);

    rb_set_red(node);
    _rb_link_color(node, tree);
}

inline void rb_replace_node(struct rb_node *victim, struct rb_node *new, struct rb_tree *tree)
{
    struct rb_node *tmp;

    *new = *victim; /* replace color and pointers */

    /* update parent pointer */
    if ((tmp = new->parent))
    {
        if (tmp->left == victim)
        {
            tmp->left = new;
        }
        else
        {
            assert(tmp->right == victim);
            tmp->right = new;
        }
    }
    else
    {
        tree->root = new;
    }

    /* update children pointers */
    if ((tmp = new->left))
    {
        tmp->parent = new;
    }

    if ((tmp = new->right))
    {
        tmp->parent = new;
    }
}

/* rebalance the tree */
static inline void _rb_remove_color(struct rb_node *node, struct rb_node *parent, struct rb_tree *tree)
{
    struct rb_node *sibling;

    assert(!parent || parent->left || parent->right);
    assert(!parent || parent->left == node || parent->right == node);

    while (rb_is_black(node) && parent)
    {
        assert(!parent || parent->left || parent->right);
        assert(!parent || parent->left == node || parent->right == node);

        if (parent->left == node)
        {
            sibling = parent->right;

            if (rb_is_red(sibling))
            {
                /* case 2 */
                _rb_rotate_left(parent, tree);
                rb_set_black(sibling);
                rb_set_red(parent);
                sibling = parent->right;
                /* continue with other cases */
            }

            if (rb_is_black(sibling->right) && rb_is_black(sibling->left))
            {
                /*
                 * case 3 and case 4
                 *   both cases paint sibling red.
                 *   however, in case 4, parent is red and painted black later, which will 
                 *   end the loop and eventually the parent will be painted red in the end
                 *   of the function.
                 */
                rb_set_red(sibling);
                node = parent;
                parent = node->parent;

                continue;
            }
            else
            {
                if (rb_is_black(sibling->right))
                {
                    /* case 5 */
                    struct rb_node *child = sibling->left;
                    _rb_rotate_right(sibling, tree);
                    rb_set_red(sibling);
                    rb_set_black(child);
                    sibling = child;
                    /* continue with case 6 */
                }

                /* case 6 */
                sibling->color = parent->color;
                rb_set_black(parent);
                assert(rb_is_red(sibling->right));
                rb_set_black(sibling->right);
                _rb_rotate_left(parent, tree);

                 /*
                  * we are going to end the loop
                  * make sure that the root node is always black in the end of the function
                  */
                node = tree->root;
                break;
            }
        }
        else
        {
            sibling = parent->left;

            if (rb_is_red(sibling))
            {
                /* case 2 */
                _rb_rotate_right(parent, tree);
                rb_set_black(sibling);
                rb_set_red(parent);

                sibling = parent->left;
            }
            
            if (rb_is_black(sibling->right) && rb_is_black(sibling->left))
            {
                /* case 3 and case 4 */
                rb_set_red(sibling);
                node = parent;
                parent = node->parent;

                continue;
            }
            else
            {
                if (rb_is_black(sibling->left))
                {
                    /* case 5 */
                    struct rb_node *child = sibling->right;
                    _rb_rotate_left(sibling, tree);
                    rb_set_red(sibling);
                    rb_set_black(child);
                    sibling = child;
                }

                /* case 6 */
                sibling->color = parent->color;
                rb_set_black(parent);
                assert(rb_is_red(sibling->left));
                rb_set_black(sibling->left);
                _rb_rotate_right(parent, tree);

                node = tree->root;
                break;
            }
        }
    }

    /* case 4, case 1 or the child of the node being removed is red */
    if (node)
    {
        rb_set_black(node);
    }
}

void rb_remove(struct rb_node *node, struct rb_tree *tree)
{
    struct rb_node *child, *parent;
    int color; /* save the color of the node being removed */

    if (!node->left || !node->right)
    {
        /* the node have no child or have one child */
        if (node->left)
        {
            child = node->left;
        }
        else
        {
            child = node->right;
        }

        color = node->color;

        /* remove the node */
        parent = node->parent;
        if (parent)
        {
            if (parent->left == node)
            {
                parent->left = child;
            }
            else
            {
                assert(parent->right == node);
                parent->right = child;
            }
        }
        else
        {
            tree->root = child;
        }

        if (child)
        {
            child->parent = parent;
        }
    }
    else
    {
        /* complex case. we need to find the successor of the node */
        struct rb_node *tmp, *old = node;

        node = old->right;

        /* find the leftmost of the node */
        while ((tmp = node->left))
        {
            node = tmp;
        }
        /* node is the successor of the old node now */
        assert(!(node->right && node->left));
        assert(!node->left);

        color = node->color; /* save the color of the node */

        /* since the node is the leftmost of the old node,
           if it has a child, it must on the right */
        child = node->right;
        parent = node->parent;

        /* remove the node */
        if (parent)
        {
            if (parent->left == node) {
                parent->left = child;
            } else {
                assert(parent->right == node);
                parent->right = child;
            }
        }
        else
        {
            tree->root = child;
        }

        if (child)
        {
            child->parent = parent;
        }

        /* replace the old node with the successor */
        if (old == parent) {
            /* If the old node is the parent.
             * We point it to the new node now */
            parent = node;
        }
        rb_replace_node(old, node, tree);
    }

    /* node is not usable here */

    if (color == RB_BLACK)
    {
        /* If the node being removed is black. We need to rebalance the tree. */
        _rb_remove_color(child, parent, tree);
    }
}

inline struct rb_node *rb_leftmost(struct rb_node *node)
{
    while (node->left)
    {
        node = node->left;
    }

    return node;
}

inline struct rb_node *rb_rightmost(struct rb_node *node)
{
    while (node->right)
    {
        node = node->right;
    }

    return node;
}

struct rb_node *rb_next(struct rb_node *node)
{
    struct rb_node *parent;

    if (node->right)
    {
        return rb_leftmost(node->right);
    }

    while ((parent = node->parent) && parent->right == node)
    {
        node = parent;
    }

    return node->parent;
}

struct rb_node *rb_prev(struct rb_node *node)
{
    struct rb_node *parent;

    if (node->left)
    {
        return rb_rightmost(node->left);
    }

    while ((parent = node->parent) && parent->left == node)
    {
        node = parent;
    }

    return node->parent;
}

struct rb_node *rb_first(struct rb_tree *tree)
{
    if (tree->root)
    {
        return rb_leftmost(tree->root);
    }

    return NULL;
}
struct rb_node *rb_last(struct rb_tree *tree)
{
    if (tree->root)
    {
        return rb_rightmost(tree->root);
    }

    return NULL;
}
