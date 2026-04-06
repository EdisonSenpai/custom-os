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
