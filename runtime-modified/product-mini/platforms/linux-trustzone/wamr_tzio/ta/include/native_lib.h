#ifndef NATIVE_LIB_H
#define NATIVE_LIB_H

#include"bh_platform.h"
#include"wasm_export.h"

struct native_lib {
    void *handle;

    uint32 (*get_native_lib)(char **p_module_name,
                             NativeSymbol **p_native_symbols);
    int (*init_native_lib)(void);
    void (*deinit_native_lib)(void);
    
    char *module_name;
    NativeSymbol *native_symbols;
    uint32 n_native_symbols;
};

struct native_lib * load_native_lib(const char *name);
uint32 load_and_register_native_libs(const char **native_lib_list,
                              uint32 native_lib_count,
                              struct native_lib **native_lib_loaded_list);


#endif /*NATIVE_LIB_H */