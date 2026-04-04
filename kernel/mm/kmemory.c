#include "mm/kmemory.h"

#include <stdint.h>

void* kmemset(void* dest, int value, uint32_t count)
{
    uint8_t* out = (uint8_t*)dest;
    uint8_t byte_value = (uint8_t)value;
    uint32_t i = 0u;

    while (i < count) {
        out[i] = byte_value;
        i++;
    }

    return dest;
}

void* kmemcpy(void* dest, const void* src, uint32_t count)
{
    uint8_t* out = (uint8_t*)dest;
    const uint8_t* in = (const uint8_t*)src;
    uint32_t i = 0u;

    while (i < count) {
        out[i] = in[i];
        i++;
    }

    return dest;
}

void* kmemmove(void* dest, const void* src, uint32_t count)
{
    uint8_t* out = (uint8_t*)dest;
    const uint8_t* in = (const uint8_t*)src;

    if (out == in || count == 0u) {
        return dest;
    }

    if (out < in || out >= (in + count)) {
        uint32_t i = 0u;

        while (i < count) {
            out[i] = in[i];
            i++;
        }
    } else {
        uint32_t i = count;

        while (i != 0u) {
            i--;
            out[i] = in[i];
        }
     }

     return dest;
 }

int kmemcmp(const void* a, const void* b, uint32_t count)
{
    const uint8_t* lhs = (const uint8_t*)a;
    const uint8_t* rhs = (const uint8_t*)b;
    uint32_t i = 0u;

    while (i < count) {
        if (lhs[i] != rhs[i]) {
            return (int)lhs[i] - (int)rhs[i];
        }

        i++;
    }

    return 0;
}
