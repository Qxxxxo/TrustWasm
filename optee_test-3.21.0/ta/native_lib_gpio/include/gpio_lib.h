/**
 * @brief native lib for gpio
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _GPIO_LIB_H_
#define _GPIO_LIB_H_

#include "wasm_export.h"

int init_native_lib();
void deinit_native_lib();
uint32_t get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols);

#endif /* _GPIO_LIB_H_ */
