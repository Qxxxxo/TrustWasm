/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

/*
 * This example basically does the same thing as test_hello.c,
 * using wasm_export.h API.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm_export.h"


#define ADDRESS_MASK_32 0x0000FFFF  
#define SIGNATURE_MASK_32 0xFF000000
#define TAG_MASK_32 0x00FF0000

uint32_t setSignatureAndTag(uint32_t pointer, uint8_t signature, uint8_t tag) {
    pointer &= ADDRESS_MASK_32;
    pointer |= ((uint32_t)signature << 24) | ((uint32_t)tag << 16);
    return pointer;
}

uint8_t getSignature(uint32_t pointer) {
    return (pointer & SIGNATURE_MASK_32) >> 24;
}

uint8_t getTag(uint32_t pointer) {
    return (pointer & TAG_MASK_32) >> 16;
}

uint32_t ClearSignatureAndTag(uint32_t pointer) {
    pointer &= ADDRESS_MASK_32;
    return pointer;
}


// static int
// test_hello2_wrapper(wasm_exec_env_t exec_env, uint32_t nameaddr,
//                     uint32_t resultaddr, uint32_t resultlen)
// {
//     int re = 101;
//     /*
//      * Perform wasm_runtime_malloc to check if the runtime has been
//      * initialized as expected.
//      * This would fail with "memory hasn't been initialize" error
//      * unless we are not sharing a runtime with the loader app. (iwasm)
//      */
//     void *p = wasm_runtime_malloc(1);
//     if (p == NULL) {
//         return -1;
//     }
//     wasm_runtime_free(p);

//     wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
//     if (!wasm_runtime_validate_app_str_addr(inst, (uint64_t)nameaddr)
//         || !wasm_runtime_validate_app_addr(inst, (uint64_t)resultaddr,
//                                            (uint64_t)resultlen)) {
//         return -1;
//     }
//     const char *name =
//         wasm_runtime_addr_app_to_native(inst, (uint64_t)nameaddr);
//     char *pp = malloc(2);
//     strcpy(pp, name);
//     return re;
// }

static int
test_hello2_wrapper(wasm_exec_env_t exec_env, 
                    uint32_t resultaddr, uint32_t resultlen)
{

    int re = 1;
    /*
     * Perform wasm_runtime_malloc to check if the runtime has been
     * initialized as expected.
     * This would fail with "memory hasn't been initialize" error
     * unless we are not sharing a runtime with the loader app. (iwasm)
     */
    void *p = wasm_runtime_malloc(1);
    if (p == NULL) {
        return -1;
    }
    wasm_runtime_free(p);

    uint8_t sign = getSignature(resultaddr);
    uint8_t tag = getTag(resultaddr);
    printf("%d, %d\n", sign, tag);
    
    resultaddr = ClearSignatureAndTag(resultaddr);

    wasm_module_inst_t inst = wasm_runtime_get_module_inst(exec_env);
    if (!wasm_runtime_validate_app_addr(inst, (uint64_t)resultaddr,
                                           (uint64_t)resultlen)) {
        return -1;
    }

    char *result = wasm_runtime_addr_app_to_native(inst, (uint64_t)resultaddr);
    printf("wasm address: %x" ", native address: %p\n", resultaddr, result);
    strcpy(result, "32.9");
    return re;
}

/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(test_hello2, "(ii)i")
};
/* clang-format on */

uint32_t
get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols)
{
    *p_module_name = "env";
    *p_native_symbols = native_symbols;
    return sizeof(native_symbols) / sizeof(NativeSymbol);
}

int
init_native_lib()
{
    printf("%s in test_hello2.c called\n", __func__);
    return 0;
}

void
deinit_native_lib()
{
    printf("%s in test_hello2.c called\n", __func__);
}
