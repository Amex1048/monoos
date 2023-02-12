#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <memory.h>
#include <devices/terminal.h>

#include <linked_list.h>

void push_back(void *obj, list_t **list)
{
    list_t *node = kmalloc(sizeof(list_t), MM_STANDART);
    node->obj = obj;

    if (*list == NULL)
    {
        *list = node;
        node->next = node;
        node->prev = node;
    }
    else
    {
        node->prev = (*list)->prev;
        node->next = *list;

        (*list)->prev->next = node;
        (*list)->prev = node;
    }
}

void push_front(void *obj, list_t **list)
{
    push_back(obj, list);
    *list = (*list)->prev;
}

void *pop_back(list_t **list)
{
    if (list == NULL)
    {
        return NULL;
    }

    void *obj = (*list)->prev->obj;

    if ((*list)->next == *list)
    {
        kfree(*list);
        *list = NULL;

        return obj;
    }

    list_t *last = (*list)->prev;

    (*list)->prev = last->prev;
    last->prev->next = *list;

    kfree(last);

    return obj;
}

void *pop_front(list_t **list)
{
    if (*list == NULL)
    {
        return NULL;
    }

    void *obj = (*list)->obj;

    if ((*list)->next == *list)
    {
        kfree(*list);
        *list = NULL;

        return obj;
    }

    list_t *last = *list;
    *list = (*list)->next;

    (*list)->prev = last->prev;
    last->prev->next = *list;

    kfree(last);

    return obj;
}

void remove(void *obj, list_t **list)
{
    if (list == NULL)
    {
        return;
    }

    list_t *node = *list;

    if (node->obj == obj)
    {
        pop_front(list);
        return;
    }

    do
    {
        if (node->obj == obj)
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            kfree(node);

            return;
        }

        node = node->next;
    } while ((*list) != node);
}

void insert_with_compare(void *obj, list_t **list, bool (*comparator)(void *, void *, void *))
{
    if (*list == NULL)
    {
        push_back(obj, list);
        return;
    }

    list_t *node = *list;

    do
    {
        if (comparator(node->prev->obj, obj, node->obj))
        {
            // Insert obj before node
            if (node == *list)
            {
                push_front(obj, list);
            }
            else
            {
                push_back(obj, &node);
            }

            return;
        }

        node = node->next;
    } while (node != *list);

    push_back(obj, &node);
}

void print_list(list_t *list)
{
    if (list == NULL)
    {
        print_string("List is empty!\n");
        return;
    }

    list_t *node = list;

    print_string("List ");
    print_dword((uint32_t)list);
    print_string(" now:\n");

    do
    {
        print_string("Node: ");
        print_dword((uint32_t)node);
        print_char('\n');

        print_string("Next: ");
        print_dword((uint32_t)node->next);

        print_string(", prev: ");
        print_dword((uint32_t)node->prev);

        print_string(", obj: ");
        print_dword((uint32_t)node->obj);

        print_string("\n||||\n");

        node = node->next;
    } while (node != list);
}
