/*
 *  list.c
 *  
 *  defines the helper functions and data type for implementing
 *  a linked list
 */

#include <ylib.h>
#include "list.h"

static lnode_t *lnode_init(void *item);

static void lnode_delete(lnode_t *node, void (*delete)(void *data));

list_t *list_init() {
    list_t *list = malloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

int list_add(list_t *list, void *item) {
    if (list == NULL) return -1;
    lnode_t *node = lnode_init(item);
    if (node == NULL) return -1;
    if (list->head == NULL) {
        list->head = node;
        list->tail = list->head;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = list->tail->next;
    }
    list->size++;
    return list->size;
}

void *list_pop(list_t *list) {
    if (list == NULL) return NULL;
    if (list->tail == NULL) return NULL;
    lnode_t *node = list->tail;
    list->tail = node->prev;
    if (list->tail == NULL) list->head = NULL;
    list->size--;
    void *data = node->data;
    lnode_delete(node, NULL);
    return data;
}

/**
 * @brief 
 * 
 * @param list 
 * @param arg 
 * @param iter 
 */
void list_iter(list_t *list, void *arg, void (*iter) (void *arg, void *item)) {
    if (list == NULL || iter == NULL) return;
    for (lnode_t *node = list->head; node != NULL; node = node->next) {
        iter(arg, (void *) node->data);
    }
}

static lnode_t *lnode_init(void *item) {
    lnode_t *node = malloc(sizeof(lnode_t));
    if (node == NULL) return NULL;
    node->data = item;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

static void lnode_delete(lnode_t *node, void (*delete)(void *data)) {
    if (node != NULL) {
        if (delete != NULL) delete(node->data);
        free(node);
    }
}

