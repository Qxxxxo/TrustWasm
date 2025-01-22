
#include "safe_op.h"

void *safe_malloc(size_t size) {
    size_t total_size = size + 2 * GUARD_SIZE; 
    unsigned int *ptr = (unsigned int *)malloc(total_size);
    if (ptr == NULL) {
        return NULL;
    }

    ptr[0] = GUARD_VALUE; 
    void *user_ptr = (void *)(ptr + 1); 
    unsigned int *end_guard = (unsigned int *)((char *)user_ptr + size); 
    *end_guard = GUARD_VALUE; 

    return user_ptr;
}

int san_guard(void *ptr, size_t size){
    if (ptr == NULL) {
        return 1;
    }

    unsigned int *start_guard = (unsigned int *)ptr - 1;

    unsigned int *end_guard = (unsigned int *)((char *)ptr + size);

    if (*start_guard != GUARD_VALUE) {
        printf("Buffer underflow detected!\n");
        return 0;
    }
    if (*end_guard != GUARD_VALUE) {
        printf("Buffer overflow detected!\n");
        return 0;
    }
    return 1;

}

void safe_free(void *ptr, size_t size) {
    if (ptr == NULL) {
        return;
    }
    unsigned int *start_guard = (unsigned int *)ptr - 1;
    unsigned int *end_guard = (unsigned int *)(ptr + size);

    if (*start_guard != GUARD_VALUE) {
        printf("Buffer underflow detected!\n");
    }
    if (*end_guard != GUARD_VALUE) {
        printf("Buffer overflow detected!\n");
    }
    free(start_guard);
}

uint32_t setSignatureAndTag(uint32_t pointer, uint8_t signature, uint8_t tag) {
    pointer &= ADDRESS_MASK_32;
    pointer |= ((uint32_t)signature << 24) | ((uint32_t)tag << 8);
    return pointer;
}

uint8_t getSignature(uint32_t pointer) {
    return (pointer & SIGNATURE_MASK_32) >> 24;
}

uint8_t getTag(uint32_t pointer) {
    return (pointer & TAG_MASK_32) >> 16;
}

void print_binary(uint32_t n) {
    for (int i = 31; i >= 0; i--) {
        printf("%u", (n >> i) & 1);
        if (i % 8 == 0) printf(" ");
    }
    printf("\n");
}
