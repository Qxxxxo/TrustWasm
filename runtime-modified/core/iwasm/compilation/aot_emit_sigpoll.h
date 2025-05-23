/**
 * @brief compilation support for async io signal polling
 * reference to WALI signal poll support
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef _AOT_EMIT_SIGPOLL_H_
#define _AOT_EMIT_SIGPOLL_H_

#include "aot_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

bool
aot_emit_sigpoll(AOTCompContext *comp_ctx, AOTFuncContext *func_ctx);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif