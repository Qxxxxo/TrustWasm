#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ADDRESS_MASK_32 0x0000FFFF  
#define SIGNATURE_MASK_32 0xFF000000
#define TAG_MASK_32 0x00FF0000

#define GUARD_VALUE 0xDEADBEEF
#define GUARD_SIZE sizeof(unsigned int)

void *safe_malloc(size_t size);

int san_guard(void *ptr, size_t size);

void safe_free(void *ptr, size_t size);

uint32_t setSignatureAndTag(uint32_t pointer, uint8_t signature, uint8_t tag);

uint8_t getSignature(uint32_t pointer);

uint8_t getTag(uint32_t pointer);

void print_binary(uint32_t n);
