#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct list_t list_t;
struct list_t
{
    list_t *prev;
    list_t *next;

    void *obj;
};

void push_front(void *obj, list_t **list);

void push_back(void *obj, list_t **list);

void insert_with_compare(void *obj, list_t **list, bool (*comparator)(void *, void *, void *));

void *pop_front(list_t **list);

void *pop_back(list_t **list);

void remove(void *obj, list_t **list);

void print_list(list_t *list);

#endif /* LINKEDLIST_H */