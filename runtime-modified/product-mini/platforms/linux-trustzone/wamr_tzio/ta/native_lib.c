#include<dlfcn.h>
#include<string.h>
#include"native_lib.h"
#include"wasm.h"
#include"logging.h"

struct native_lib *
load_native_lib(const char *name)
{
    struct native_lib *lib = wasm_runtime_malloc(sizeof(*lib));
    if (lib == NULL) {
        IMSG("warning: failed to load native library %s because of "
                    "allocation failure",
                    name);
        goto fail;
    }
    memset(lib, 0, sizeof(*lib));
    /* open the native library */
    if (!(lib->handle = dlopen(name, (RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE)))) {
        IMSG("warning: failed to load native library %s.", name);
        goto fail;
    }

    lib->init_native_lib = dlsym(lib->handle, "init_native_lib");
    lib->get_native_lib = dlsym(lib->handle, "get_native_lib");
    lib->deinit_native_lib = dlsym(lib->handle, "deinit_native_lib");

    if (!lib->get_native_lib) {
        IMSG("warning: failed to lookup `get_native_lib` function "
                    "from native lib %s",
                    name);
        goto fail;
    }

    if (lib->init_native_lib) {
        int ret = lib->init_native_lib();
        if (ret != 0) {
            IMSG("warning: `init_native_lib` function from native "
                        "lib %s failed with %d",
                        name, ret);
            goto fail;
        }
    }

    lib->n_native_symbols =
        lib->get_native_lib(&lib->module_name, &lib->native_symbols);

    /* register native symbols */
    if (!(lib->n_native_symbols > 0 && lib->module_name && lib->native_symbols
          && wasm_runtime_register_natives(
              lib->module_name, lib->native_symbols, lib->n_native_symbols))) {
        IMSG("warning: failed to register native lib %s", name);
        if (lib->deinit_native_lib) {
            lib->deinit_native_lib();
        }
        goto fail;
    }
    return lib;
fail:
    if (lib != NULL) {
        if (lib->handle != NULL) {
            dlclose(lib->handle);
        }
        IMSG("fail free");
        wasm_runtime_free(lib);
    }
    return NULL;
}

uint32
load_and_register_native_libs(const char **native_lib_list,
                              uint32 native_lib_count,
                              struct native_lib **native_lib_loaded_list)
{
    uint32 i, native_lib_loaded_count = 0;

    for (i = 0; i < native_lib_count; i++) {
        struct native_lib *lib = load_native_lib(native_lib_list[i]);
        if (lib == NULL) {
            continue;
        }
        native_lib_loaded_list[native_lib_loaded_count++] = lib;
    }

    return native_lib_loaded_count;
}