#include "tz_io_sigtable.h"
#include <tee_internal_api.h>
#include "wasm_runtime.h"
#include "wasm_export.h"


// these externs are very important for compiling wamrc
extern TEE_Result TEE_OpenTASession(const TEE_UUID *destination, 
                                    uint32_t cancellationRequestTimeout, 
                                    uint32_t paramTypes, 
                                    TEE_Param params[TEE_NUM_PARAMS], 
                                    TEE_TASessionHandle *session, 
                                    uint32_t *returnOrigin) __attribute__((weak));

extern TEE_Result TEE_InvokeTACommand(TEE_TASessionHandle session, 
                                      uint32_t cancellationRequestTimeout,
                                      uint32_t commandID, 
                                      uint32_t paramTypes, 
                                      TEE_Param params[TEE_NUM_PARAMS], 
                                      uint32_t *returnOrigin) __attribute__((weak));

extern void trace_printf(const char *func, int line, int level, bool level_ok,
		  const char *fmt, ...) __attribute__((weak));
    
// for test poll only
extern void TEE_GetSystemTime(TEE_Time * t) __attribute__((weak));


// timeout after 100 ms
#define IO_HELPER_TIME_OUT TEE_TIMEOUT_INFINITE

#define FUNC_IDX(func) ({ wasm_runtime_get_function_idx(module_inst, func); })

uint64_t profile_times=0;

#if USE_ADAPTIVE_CHECKPOINT != 0

float alert_coef=0.9;

exec_time_t exec_time_table[N_REQ_TYPE]={
    {0,0,0},
    {0,0,0},
    {0,5600,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
};

int64_t wait_profile=0;
bool no_profile=false;

io_req_queue io_queues[N_SIG]={0};

bool is_empty(io_req_queue * q){
    return q->front==NULL;
}

int32_t enqueue(io_req_queue * q, io_req_t req){
    io_req_node * new_node = (io_req_node *)malloc(sizeof(io_req_node));
    if(new_node==NULL){
        DMSG("Memory alloc failed");
        return -1;
    }
    new_node->req=req;
    new_node->next=NULL;
    if(is_empty(q)){
        q->front=new_node;
        q->rear=new_node;
    }else{
        q->rear->next=new_node;
        q->rear=new_node;
    }
    return 0;
}

int32_t dequeue(io_req_queue * q, io_req_t * req){
    if(is_empty(q)){
        DMSG("Queue is empty");
        return -1;
    }
    io_req_node * tmp= q->front;
    *req=tmp->req;
    q->front=q->front->next;
    if(q->front==NULL) q->rear=NULL;
    free(tmp);
    return 0;
}

bool profile(){
    TEE_Time now_time;
    uint64_t now;
    TEE_GetSystemTime(&now_time);
    now=teetime_to_micro(now_time);
    profile_times++;
    int idx = __builtin_ffsll(io_sigpending);
    
        // if(io_queues[i].front!=NULL)
        //     DMSG("now %llu, profile on periph_no %d, type %d, start %llu, alert %llu",now,i,io_queues[i].front->req.type,io_queues[i].front->req.start,
        //     (uint64_t)(alert_coef*(float)(exec_time_table[io_queues[i].front->req.type].k*
        //     (float)io_queues[i].front->req.n+exec_time_table[io_queues[i].front->req.type].b)));

        // (now-start)>α·(kn+b)
        // if((io_queues[i].front!=NULL)
        //     &&((now-io_queues[i].front->req.start)>
        //     (uint64_t)((exec_time_table[io_queues[i].front->req.type].k*
        //     (float)io_queues[i].front->req.n+exec_time_table[io_queues[i].front->req.type].b))))
        //     return true;
    if((io_queues[idx-1].front!=NULL)
        &&(now>io_queues[idx-1].front->req.guess_end)){
        no_profile=true;
        return true;
    }else if(io_queues[idx-1].front!=NULL){
        wait_profile=exec_time_table[io_queues[idx-1].front->req.type].wait_profile;
    }
    
    return false;
}

#endif

io_sigentry io_sigtable[N_SIG][N_SUB_SIG]={0};

uint64_t io_sigpending=0;
uint64_t recover_io_sigpending=0;
// check if the program is in a handler func
int8_t in_handler=0;

// for test
uint64_t poll_times=0;

#if USE_PTA_IO_HELPER != 0

#define PTA_IO_HELPER_CMD_POLL 1

static TEE_TASessionHandle io_helper_sess=TEE_HANDLE_NULL;

static TEE_Result invoke_io_helper_pta(uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS]){
        TEE_Result res = TEE_ERROR_GENERIC;
        if(io_helper_sess==TEE_HANDLE_NULL){
            // open session
            TEE_UUID uuid=PTA_IO_HELPER_UUID;
            // get io helper uuid
            res=TEE_OpenTASession(&uuid,IO_HELPER_TIME_OUT,0,NULL,&io_helper_sess ,NULL);
            // return error
            if(res!=TEE_SUCCESS) {
                EMSG("The session with the io helper cannot be established. Error: %x",res);
                return res;
            }
        }
        return TEE_InvokeTACommand(io_helper_sess ,IO_HELPER_TIME_OUT,cmd_id,param_types,params,NULL);
}

#else

#define TA_IO_HELPER_CMD_POLL 1

static TEE_TASessionHandle io_helper_sess[N_SIG]={TEE_HANDLE_NULL};

static TEE_Result invoke_io_helper_ta(uint32_t sess_no, uint32_t cmd_id, uint32_t param_types,
                                       TEE_Param params[TEE_NUM_PARAMS]){
        TEE_Result res = TEE_ERROR_GENERIC;
        // load existed session
        if(io_helper_sess[sess_no]==TEE_HANDLE_NULL){
            // open session
            TEE_UUID uuid={};
            // get io helper uuid
            tzio_signo_to_io_helper_uuid(sess_no,&uuid);
            res=TEE_OpenTASession(&uuid,IO_HELPER_TIME_OUT,0,NULL,&io_helper_sess[sess_no] ,NULL);
            // return error
            if(res!=TEE_SUCCESS) {
                EMSG("The session with the io helper cannot be established. Error: %x",res);
                return res;
            }
        }
        return TEE_InvokeTACommand(io_helper_sess[sess_no] ,IO_HELPER_TIME_OUT,cmd_id,param_types,params,NULL);                              
}
#endif

static TEE_Result poll_to_io_helper(uint32_t sess_no, uint32_t * ready_sub_signo){
    TEE_Result res;
#if USE_PTA_IO_HELPER != 0
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);\
    TEE_Param params[TEE_NUM_PARAMS] = {};
    // for now sess_no is periph no
    uint16_t periph_no=(uint16_t)sess_no;
    params[0].value.a=periph_no;
    res=invoke_io_helper_pta(PTA_IO_HELPER_CMD_POLL,param_types,params);                             
#else
    uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE,
                                  TEE_PARAM_TYPE_NONE);
    TEE_Param params[TEE_NUM_PARAMS] = {};
    res=invoke_io_helper_ta(sess_no,TA_IO_HELPER_CMD_POLL,param_types,params);
#endif
    if(res!=TEE_SUCCESS){
        EMSG("poll to signo(%d) io helper failed",sess_no);
        return TEE_ERROR_COMMUNICATION;
    }
    // io helper request done
    // return ready sub-signo
    if(params[0].value.a==0){
        // no io request
        return TEE_ERROR_NO_DATA;
    } else{
        // a>0, still have processing or done request
        // return done request sub-signo
        if(params[0].value.b==0xFFFFFFFF){
            // no done request
            return TEE_ERROR_BUSY;
        }else if(params[0].value.b<N_SUB_SIG){
            *ready_sub_signo=params[0].value.b;
            return TEE_SUCCESS;
        }
        return TEE_ERROR_GENERIC;
    }
}

int32_t tzio_periph_to_io_signal_no(uint32_t periph_type,uint32_t periph_no){
    if(periph_type>(uint32_t)PERIPH_SECURE_STORAGE) return -1;
    if((periph_type_t)periph_type==PERIPH_GPIO){
        // mask low 6 bit, less than 64
        return (int32_t)(periph_no&0x3f);
    }else if((periph_type_t)periph_type==PERIPH_I2C){
        return (int32_t)((periph_no+28)&0x3f); // after availbale 28 gpios 
    } else if((periph_type_t)periph_type==PERIPH_SPI){
        return (int32_t)((periph_no+30)&0x3f); // after availbale 28 gpios and 2 i2c
    } else if((periph_type_t)periph_type==PERIPH_SECURE_STORAGE){
        return (int32_t)((periph_no+31)&0x3f); // after availabel 28 gpio, 2 i2c, 1 spi
    }
    return -1;
}

int get_ready_signal(uint32_t * ready_signo,uint32_t* ready_sub_signo){
    if(io_sigpending){
       
        // for test poll only
        // TEE_Time start,end;
        // TEE_GetSystemTime(&start);
        uint64_t tmp_io_sigpending=io_sigpending;
        // since runtime TA is single-threaded, we don't need mutex
        while(tmp_io_sigpending){
            int idx = __builtin_ffsll(tmp_io_sigpending);
            // idx should >= 1
            // clear tmp pending bit
            tmp_io_sigpending &= ~(((uint64_t)1<<idx)-1);
            TEE_Result res = poll_to_io_helper(idx-1,ready_sub_signo);
            if(res==TEE_SUCCESS){
                // find ready signo
                *ready_signo=idx-1;
                // DMSG("ready signo %d, ready sub signo %d",*ready_signo,*ready_sub_signo);
                return 1;
            }else if(res==TEE_ERROR_NO_DATA){
                // all request done, clear pending bit
                // DMSG("signo %d all request done",idx-1);
                io_sigpending &= ~((uint64_t)1<<(idx-1));
            }
            // check next pending signal
        }
        // for test poll only
        // TEE_GetSystemTime(&end);
        // DMSG("poll interval: %lu",(end.seconds-start.seconds)*1000000+(end.micros-start.micros));
        // if io helper's signal no pendig
        return 0;
    }else{
        return -1;
    }
}

int32_t all_signal_handled(void){
    return (io_sigpending==0);
}

// for test
void tzio_reset_poll_times(void){
    poll_times=0;
}

uint64_t tzio_get_poll_times(void){
    return poll_times;
}

void tzio_reset_profile_times(void){
    profile_times=0;
}

uint64_t tzio_get_profile_times(void){
    return profile_times;
}

static inline void set_signal_pending(uint32_t signo){
    // when in a handler func, all signal pending will be reserved
    // and recover when out of the handler func,
    // so you can send async req in a handler func, 
    // but the handler can only be triggered out of the handler func 
    if(in_handler>0){
        recover_io_sigpending |= ((uint64_t)1UL<<signo);
    }else{
        io_sigpending |= ((uint64_t)1UL<<signo);
    }
}

void temp_unset_all_io_signal_pending(){
    recover_io_sigpending=io_sigpending;
    io_sigpending=0;
    in_handler=1;
}

void temp_recover_all_io_signal_pending(){
    in_handler=0;
    io_sigpending=recover_io_sigpending;
}



int32_t tzio_register_handler(wasm_exec_env_t exec_env, uint32_t signo, uint32_t sub_signo, uint32_t handler_func_ptr, uint32_t req_type, uint32_t len){
    if(signo>N_SIG||sub_signo>N_SUB_SIG) return -1;
    if(handler_func_ptr==0) return -1;
    if(exec_env==NULL) return -1;
#if USE_ADAPTIVE_CHECKPOINT != 0
    if(req_type>N_REQ_TYPE) return -1;
    TEE_Time start;
    TEE_GetSystemTime(&start);
    io_req_t req={
        (io_req_type_t)req_type,
        teetime_to_micro(start), // start
        teetime_to_micro(start)+exec_time_table[req_type].k*len+exec_time_table[req_type].b, // guess end
        len
    };
    enqueue(&io_queues[signo],req);
#endif
    wasm_module_inst_t module_inst = get_module_inst(exec_env);
    wasm_function_inst_t target_wasm_handler = wasm_runtime_get_indirect_function(
                                                module_inst,0,handler_func_ptr);
    uint32_t func_idx = target_wasm_handler ? FUNC_IDX(target_wasm_handler) : 0;
    uint32_t old_func_idx = io_sigtable[signo][sub_signo].function ? FUNC_IDX(io_sigtable[signo][sub_signo].function):0;
    // DMSG("replace signo %d sub signo %d, target handler func %d -> func %d",signo,sub_signo, old_func_idx,func_idx);
    // replace old func
    if(io_sigtable[signo][sub_signo].function&&module_inst->module_type == Wasm_Module_AoT){
        // aot func free
        wasm_runtime_free(io_sigtable[signo][sub_signo].function);
    }
    io_sigtable[signo][sub_signo].function = target_wasm_handler;
    io_sigtable[signo][sub_signo].func_table_idx = handler_func_ptr;
    io_sigtable[signo][sub_signo].func_idx = func_idx;
    // set signo pending
    set_signal_pending(signo);
    return 0;
}