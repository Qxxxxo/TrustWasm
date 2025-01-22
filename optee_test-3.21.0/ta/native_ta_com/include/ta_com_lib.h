/** 
 *  Author: Quakso
 *  Email: XXXXX@XXXXXX
 *  Date: 2024-11-09
 *  Version: 1.0
 *  Description: 
 */
#ifndef _TA_COM_LIB_H_
#define _TA_COM_LIB_H_

#include "wasm_export.h"

int init_native_lib();
void deinit_native_lib();
uint32_t get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols);

#endif /* _TA_COM_LIB_H_ */
