/**
 * @brief sigtabel
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#ifndef _TZ_IO_SIGTABLE_H_
#define _TZ_IO_SIGTABLE_H_
#include "wasm_export.h" 
#include "tz_periph_io_helper.h"
#include "pta_io_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIG_GPIO_0  0
#define SIG_GPIO_1  1
#define SIG_GPIO_22 22
#define N_SIG 64
#define N_SUB_SIG 64




#if USE_ADAPTIVE_CHECKPOINT != 0
#define teetime_to_micro(t) \
    (uint64_t)t.seconds * 1000 * 1000 + (uint64_t)t.micros
#define N_REQ_TYPE  28


typedef struct {
    io_req_type_t type;
    uint64_t start;
    uint64_t guess_end;
    uint32_t len;
} io_req_t;

typedef struct io_req_node{
    io_req_t req;
    struct io_req_node *next;
}io_req_node;

typedef struct {
    io_req_node * front;
    io_req_node * rear;
}io_req_queue;

typedef struct {
    uint64_t k;
    uint64_t b;
    int64_t wait_profile;
} exec_time_t;

extern int64_t wait_profile;
extern bool no_profile;
extern float alert_coef;
extern uint64_t profile_times;
extern exec_time_t exec_time_table[N_REQ_TYPE];
extern io_req_queue io_queues[N_SIG];
bool is_empty(io_req_queue * q);
int32_t enqueue(io_req_queue * q, io_req_t req);
int32_t dequeue(io_req_queue * q, io_req_t * req);
// return ture, skip poll to io helper
bool profile();
#endif

typedef struct io_sigentry{
    wasm_function_inst_t function;
    /* Index in the table, required to return back to old_sigaction */
    uint32_t func_table_idx;
    /* Index in function space of the handler */
    uint32_t func_idx;
}io_sigentry;

extern io_sigentry io_sigtable[][N_SUB_SIG];

// 64 io periphs signals
extern uint64_t io_sigpending;

// should this be device specific?
int32_t tzio_periph_to_io_signal_no(uint32_t periph_type,uint32_t periph_no);

/**
 * @brief register handler to sigtabel with sub-signo
 * @retval 0 success 
 * @retval <0 failed 
 */
int32_t tzio_register_handler(wasm_exec_env_t exec_env, uint32_t signo, uint32_t sub_signo, uint32_t handler_func_ptr, uint32_t req_type, uint32_t len);

int get_ready_signal(uint32_t * ready_signo,uint32_t* ready_sub_signo);

void temp_unset_all_io_signal_pending();
void temp_recover_all_io_signal_pending();

int all_signal_handled(void);

// for test
extern uint64_t poll_times;
void tzio_reset_poll_times(void);
uint64_t tzio_get_poll_times(void);
void tzio_reset_profile_times(void);
uint64_t tzio_get_profile_times(void);

#ifdef __cplusplus
}
#endif

#endif