#include "kernel/kobject.h"

#include "mm/kmalloc.h"

struct kobject_node* kobject_node_create(uint32_t id, uint32_t value)
{
    struct kobject_node* node = (struct kobject_node*)kmalloc((uint32_t)sizeof(struct kobject_node));

    if (node == (struct kobject_node*)0) {
        return (struct kobject_node*)0;
    }

    node->id = id;
    node->value = value;
    node->next = (struct kobject_node*)0;

    return node;
}

int kobject_node_destroy(struct kobject_node* node)
{
    if (node == (struct kobject_node*)0) {
        return 0;
    }

    return kfree((void*)node);
}

void kobject_list_init(struct kobject_list* list)
{
    if (list == (struct kobject_list*)0) {
        return;
    }

    list->head = (struct kobject_node*)0;
    list->tail = (struct kobject_node*)0;
    list->count = 0u;
}

int kobject_list_append(struct kobject_list* list, struct kobject_node* node)
{
    if (list == (struct kobject_list*)0 || node == (struct kobject_node*)0) {
        return 0;
    }

    if (node->next != (struct kobject_node*)0) {
        return 0;
    }

    if (list->head == (struct kobject_node*)0) {
        list->head = node;
        list->tail = node;
        list->count = 1u;
        return 1;
    }

    if (list->tail == (struct kobject_node*)0) {
        return 0;
    }

    list->tail->next = node;
    list->tail = node;
    list->count++;

    return 1;
}

struct kobject_node* kobject_list_find_by_id(const struct kobject_list* list, uint32_t id)
{
    struct kobject_node* cursor = (struct kobject_node*)0;

    if (list == (const struct kobject_list*)0) {
        return (struct kobject_node*)0;
    }

    cursor = list->head;

    while (cursor != (struct kobject_node*)0) {
        if (cursor->id == id) {
            return cursor;
        }

        cursor = cursor->next;
    }

    return (struct kobject_node*)0;
}

uint32_t kobject_list_count(const struct kobject_list* list)
{
    if (list == (const struct kobject_list*)0) {
        return 0u;
    }

    return list->count;
}

int kobject_list_remove_by_id(struct kobject_list* list, uint32_t id)
{
    struct kobject_node* prev = (struct kobject_node*)0;
    struct kobject_node* cursor = (struct kobject_node*)0;

    if (list == (struct kobject_list*)0) {
        return 0;
    }

    cursor = list->head;

    while (cursor != (struct kobject_node*)0) {
        if (cursor->id == id) {
            if (prev == (struct kobject_node*)0) {
                list->head = cursor->next;
            } else {
                prev->next = cursor->next;
            }

            if (list->tail == cursor) {
                list->tail = prev;
            }

            cursor->next = (struct kobject_node*)0;

            if (list->count > 0u) {
                list->count--;
            }

            if (list->count == 0u) {
                list->head = (struct kobject_node*)0;
                list->tail = (struct kobject_node*)0;
            }

            return 1;
        }

        prev = cursor;
        cursor = cursor->next;
    }

    return 0;
}
