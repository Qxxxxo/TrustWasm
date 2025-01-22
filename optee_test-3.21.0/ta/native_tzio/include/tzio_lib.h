/**
 * @brief native lib for trust zone io
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _TZIO_LIB_H_
#define _TZIO_LIB_H_

#include "wasm_export.h"


int init_native_lib();
void deinit_native_lib();
uint32_t get_native_lib(char **p_module_name, NativeSymbol **p_native_symbols);

#endif /* _TZ_LIB_H_ */
