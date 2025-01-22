/** 
 *  Author: Quakso
 *  Date: 2024-09-15
 *  Version: 1.0
 *  Description: Test wamr native lib
 */

#include<stdio.h>
#include<stdlib.h>

#include"test_add.h"

static int
test_add_wrapper(wasm_exec_env_t exec_env, int x, int y)
{
    return x + y;
}

/* clang-format off */
#define REG_NATIVE_FUNC(func_name, signature) \
    { #func_name, func_name##_wrapper, signature, NULL }

static NativeSymbol native_symbols[] = {
    REG_NATIVE_FUNC(test_add, "(ii)i")
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
    printf("%s in test_add.c called\n", __func__);
    return 0;
}

void
deinit_native_lib()
{
    printf("%s in test_add.c called\n", __func__);
}
