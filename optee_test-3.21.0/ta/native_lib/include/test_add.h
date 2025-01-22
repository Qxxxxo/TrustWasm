/** 
 *  Author: Quakso
 *  Date: 2024-09-15
 *  Version: 1.0
 *  Description: Test wamr native lib
 */
#ifndef _TEST_ADD_H_
#define _TEST_ADD_H_

#include "wasm_export.h"

int init_native_lib();
void deinit_native_lib();
uint32_t get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols);

#endif /* _TEST_ADD_H_ */
