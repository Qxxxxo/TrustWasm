/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _WASM_INTERP_H
#define _WASM_INTERP_H

#include "wasm.h"
#include "tz_io_sigtable.h"

// for test poll interval only
#include <tee_api.h>

extern void trace_printf(const char *func, int line, int level, bool level_ok,
		  const char *fmt, ...) __attribute__((weak));
extern void TEE_GetSystemTime(TEE_Time * t) __attribute__((weak));


// added for async io
// io signal handling poll
// since TA is single threaded, we don't need mutex
#if TZIO_ENABLE_INTERP_POLL !=0

#if USE_ADAPTIVE_CHECKPOINT != 0

#define HANDLE_IO_SIGNAL() \
  if(!no_profile){  \
    if(wait_profile<=0){  \
      if(profile()){  \
        if(get_ready_signal(&signo[0],&signo[1])==1){ \
          io_req_t req; \
          TEE_Time start; \
          TEE_GetSystemTime(&start);  \
          uint64_t now=teetime_to_micro(start); \
          dequeue(&io_queues[signo[0]],&req); \
          exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
          exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
          if(io_queues[signo[0]].front!=NULL){  \
              io_queues[signo[0]].front->req.start=now; \
              io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
          } \
          WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
          temp_unset_all_io_signal_pending(); \
          wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
          temp_recover_all_io_signal_pending(); \
          get_ready_signal(&signo[0],&signo[1]);  \
          no_profile=false; \
          wait_profile=0; \
        } \
      } \
    }else{  \
      wait_profile--; \
    } \
  }else{  \
    if(get_ready_signal(&signo[0],&signo[1])==1){ \
      io_req_t req; \
      TEE_Time start; \
      TEE_GetSystemTime(&start);  \
      uint64_t now=teetime_to_micro(start); \
      dequeue(&io_queues[signo[0]],&req); \
      exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
      exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
      if(io_queues[signo[0]].front!=NULL){  \
          io_queues[signo[0]].front->req.start=now; \
          io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
      } \
      WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
      temp_unset_all_io_signal_pending(); \
      wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
      temp_recover_all_io_signal_pending(); \
      get_ready_signal(&signo[0],&signo[1]);  \
      no_profile=false; \
      wait_profile=0; \
    } \
  } \

#else

#define HANDLE_IO_SIGNAL() \
  if(get_ready_signal(&signo[0],&signo[1])==1) \
  { \
    WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
    temp_unset_all_io_signal_pending(); \
    wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
    temp_recover_all_io_signal_pending(); \
  } \

#endif
  

#else
#define HANDLE_IO_SIGNAL() 

#endif

#if TZIO_ENABLE_INTERP_POLL_AT_LOOP !=0


#if USE_ADAPTIVE_CHECKPOINT != 0

#define HANDLE_IO_SIGNAL_AT_LOOP() \
  if(!no_profile){  \
    if(wait_profile<=0){  \
      if(profile()){  \
        if(get_ready_signal(&signo[0],&signo[1])==1){ \
          io_req_t req; \
          TEE_Time start; \
          TEE_GetSystemTime(&start);  \
          uint64_t now=teetime_to_micro(start); \
          dequeue(&io_queues[signo[0]],&req); \
          exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
          exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
          if(io_queues[signo[0]].front!=NULL){  \
              io_queues[signo[0]].front->req.start=now; \
              io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
          } \
          WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
          temp_unset_all_io_signal_pending(); \
          wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
          temp_recover_all_io_signal_pending(); \
          get_ready_signal(&signo[0],&signo[1]);  \
          no_profile=false; \
          wait_profile=0; \
        } \
      } \
    }else{  \
      wait_profile--; \
    } \
  }else{  \
    if(get_ready_signal(&signo[0],&signo[1])==1){ \
      io_req_t req; \
      TEE_Time start; \
      TEE_GetSystemTime(&start);  \
      uint64_t now=teetime_to_micro(start); \
      dequeue(&io_queues[signo[0]],&req); \
      exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
      exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
      if(io_queues[signo[0]].front!=NULL){  \
          io_queues[signo[0]].front->req.start=now; \
          io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
      } \
      WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
      temp_unset_all_io_signal_pending(); \
      wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
      temp_recover_all_io_signal_pending(); \
      get_ready_signal(&signo[0],&signo[1]);  \
      no_profile=false; \
      wait_profile=0; \
    } \
  } \

#else

#define HANDLE_IO_SIGNAL_AT_LOOP() \
  if(get_ready_signal(&signo[0],&signo[1])==1) \
  { \
    WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
    temp_unset_all_io_signal_pending(); \
    wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
    temp_recover_all_io_signal_pending(); \
  } \

#endif
  

#else
#define HANDLE_IO_SIGNAL_AT_LOOP() 

#endif


#if TZIO_ENABLE_INTERP_POLL_AT_BR !=0


#if USE_ADAPTIVE_CHECKPOINT != 0

#define HANDLE_IO_SIGNAL_AT_BR() \
  if(!no_profile){  \
    if(wait_profile<=0){  \
      if(profile()){  \
        if(get_ready_signal(&signo[0],&signo[1])==1){ \
          io_req_t req; \
          TEE_Time start; \
          TEE_GetSystemTime(&start);  \
          uint64_t now=teetime_to_micro(start); \
          dequeue(&io_queues[signo[0]],&req); \
          exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
          exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
          if(io_queues[signo[0]].front!=NULL){  \
              io_queues[signo[0]].front->req.start=now; \
              io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
          } \
          WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
          temp_unset_all_io_signal_pending(); \
          wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
          temp_recover_all_io_signal_pending(); \
          get_ready_signal(&signo[0],&signo[1]);  \
          no_profile=false; \
          wait_profile=0; \
        } \
      } \
    }else{  \
      wait_profile--; \
    } \
  }else{  \
    if(get_ready_signal(&signo[0],&signo[1])==1){ \
      io_req_t req; \
      TEE_Time start; \
      TEE_GetSystemTime(&start);  \
      uint64_t now=teetime_to_micro(start); \
      dequeue(&io_queues[signo[0]],&req); \
      exec_time_table[req.type].b=(uint64_t)((float)(now-req.start)*0.76+(float)exec_time_table[req.type].b*0.01);  \
      exec_time_table[req.type].wait_profile=(uint64_t)((float)tzio_get_profile_times()*0.8); \
      if(io_queues[signo[0]].front!=NULL){  \
          io_queues[signo[0]].front->req.start=now; \
          io_queues[signo[0]].front->req.guess_end=now+exec_time_table[io_queues[signo[0]].front->req.type].k*io_queues[signo[0]].front->req.len+exec_time_table[io_queues[signo[0]].front->req.type].b;  \
      } \
      WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
      temp_unset_all_io_signal_pending(); \
      wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
      temp_recover_all_io_signal_pending(); \
      get_ready_signal(&signo[0],&signo[1]);  \
      no_profile=false; \
      wait_profile=0; \
    } \
  } \

#else

#define HANDLE_IO_SIGNAL_AT_BR() \
  if(get_ready_signal(&signo[0],&signo[1])==1) \
  { \
    WASMFunctionInstance * handler_func=module->functions+io_sigtable[signo[0]][signo[1]].func_idx; \
    temp_unset_all_io_signal_pending(); \
    wasm_runtime_call_wasm(exec_env,handler_func,2,signo); \
    temp_recover_all_io_signal_pending(); \
  } \

#endif
  

#else
#define HANDLE_IO_SIGNAL_AT_BR() 

#endif

#ifdef __cplusplus
extern "C" {
#endif

struct WASMModuleInstance;
struct WASMFunctionInstance;
struct WASMExecEnv;

typedef struct WASMInterpFrame {
  /* The frame of the caller that are calling the current function. */
  struct WASMInterpFrame *prev_frame;

  /* The current WASM function. */
  struct WASMFunctionInstance *function;

  /* Instruction pointer of the bytecode array.  */
  uint8 *ip;

#if WASM_ENABLE_PERF_PROFILING != 0
  uint64 time_started;
#endif

#if WASM_ENABLE_FAST_INTERP != 0
  /* return offset of the first return value of current frame.
    the callee will put return values here continuously */
  uint32 ret_offset;
  uint32 *lp;
  uint32 operand[1];
#else
  /* Operand stack top pointer of the current frame.  The bottom of
     the stack is the next cell after the last local variable.  */
  uint32 *sp_bottom;
  uint32 *sp_boundary;
  uint32 *sp;

  WASMBranchBlock *csp_bottom;
  WASMBranchBlock *csp_boundary;
  WASMBranchBlock *csp;

  /* Frame data, the layout is:
     lp: param_cell_count + local_cell_count
     sp_bottom to sp_boundary: stack of data
     csp_bottom to csp_boundary: stack of block
     ref to frame end: data types of local vairables and stack data
     */
  uint32 lp[1];
#endif
} WASMInterpFrame;

/**
 * Calculate the size of interpreter area of frame of a function.
 *
 * @param all_cell_num number of all cells including local variables
 * and the working stack slots
 *
 * @return the size of interpreter area of the frame
 */
static inline unsigned
wasm_interp_interp_frame_size(unsigned all_cell_num)
{
  return align_uint((uint32)offsetof(WASMInterpFrame, lp)
                    + all_cell_num * 5, 4);
}

void
wasm_interp_call_wasm(struct WASMModuleInstance *module_inst,
                      struct WASMExecEnv *exec_env,
                      struct WASMFunctionInstance *function,
                      uint32 argc, uint32 argv[]);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_INTERP_H */
