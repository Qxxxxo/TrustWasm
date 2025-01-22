#ifndef WASM_H
#define WASM_H

#include <tee_internal_api.h>

#include "wasm_export.h"

typedef struct wamr_context_
{
    uint8_t *wasm_bytecode;
    uint32_t wasm_bytecode_size;
    void *heap_buf;
    uint32_t heap_size;
    wasm_module_t module;
    wasm_module_inst_t module_inst;
} wamr_context;

extern wamr_context *singleton_wamr_context;

void TA_SetOutputBuffer(void *output_buffer, uint64_t output_buffer_size);
TEE_Result TA_InitializeWamrRuntime(wamr_context* context, int argc, char** argv);
TEE_Result TA_ExecuteWamrRuntime(wamr_context* context);
void TA_TearDownWamrRuntime(wamr_context* context);

#endif /* WASM_H */