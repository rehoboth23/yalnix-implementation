#ifndef __LIST_
#define __LIST_

typedef struct lnode {
    void *data;
    struct lnode *next;
    struct lnode *prev;
} lnode_t;

typedef struct list {
    lnode_t *head;
    lnode_t *tail;
    int size;
} list_t;

list_t *list_init();

int list_add(list_t *list, void *item);

/**
 * @brief 
 * 
 * @param list 
 * @param arg 
 * @param iter 
 */
void list_iter(list_t *list, void *arg, void (*iter) (void *arg, void *item));

void *list_pop(list_t *list);

int list_remove(list_t *list, void *item, void (*check)(void *arg));

#endif