
#include "ylib.h"
#include "ykernel.h"
#include "yuser.h"

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

static lnode_t *lnode_init(void *item);
static void lnode_delete(lnode_t *node, void (*delete)(void *data));
list_t *list_init();
int list_add(list_t *list, void *item);
void *list_pop(list_t *list);

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
// TODO: simplify into just malloc a ton.
int main(int argc, char const *argv[]) {
    int count = 0;
    int mal = 1;
    int mal_limit = 2;
    int mal_size = 100000;
    list_t *list = list_init();
    int pid = GetPid();
    int ppid = 0;
    while (1) {
        TracePrintf(1, "brk.c: PID -> %d\tPPID -> %d\tMalloc Count -> %d\n", pid, ppid, count);
        if (mal) {
            list_add(list, malloc(mal_size));
            count++;
            if (count == mal_limit) {
                mal = 0;
                count--;
            }
        } else {
            // free(list_pop(list));
            // count--;
            // if (count == 0) mal = 1;
            // Pause();
            // Halt();
        }
        // Delay(1);
        Pause();
    }
    return 0;
}
