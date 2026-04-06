#ifndef EA3D8A4D_8D83_45E6_91C9_765C84A7EA95
#define EA3D8A4D_8D83_45E6_91C9_765C84A7EA95
#ifndef CUSTOM_OS_KERNEL_KOBJECT_H
#define CUSTOM_OS_KERNEL_KOBJECT_H

#include <stdint.h>

struct kobject_node {
    uint32_t id;
    uint32_t value;
    struct kobject_node* next;
};

struct kobject_list {
    struct kobject_node* head;
    struct kobject_node* tail;
    uint32_t count;
};

struct kobject_node* kobject_node_create(uint32_t id, uint32_t value);
int kobject_node_destroy(struct kobject_node* node);
void kobject_list_init(struct kobject_list* list);
int kobject_list_append(struct kobject_list* list, struct kobject_node* node);

#endif


#endif /* EA3D8A4D_8D83_45E6_91C9_765C84A7EA95 */
