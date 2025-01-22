/**
 * @brief io signal
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef IO_SIGNAL_H
#define IO_SIGNAL_H
#include <stdint.h>
#include <kernel/mutex.h>
#include <trace.h>

#define N_PERIPH  64
#define N_SUB_SIG 64

typedef struct io_sigentry_t
{
    int ret;      // return value for io request
    void *buf;    // for read req save the out data
    uint32_t len; // out data len
    uint32_t poll_times; // for test poll 
} io_sigentry_t;

// read consistency/modify use sigpending mutexes
extern uint64_t io_helper_sigpending[N_PERIPH];
extern uint32_t poll_times[N_PERIPH];
extern struct mutex sigpending_mutexes[N_PERIPH];

// read consistency/modify use sigusing mutexes
extern uint64_t io_helper_sigusing[N_PERIPH];
// req_nums is the number of signal using
// modify use sigusing mutex
extern uint32_t req_nums[N_PERIPH];

extern struct mutex sigusing_mutexes[N_PERIPH];


extern io_sigentry_t io_helper_signal_table[N_PERIPH][N_SUB_SIG];

/**
 * @brief get ready signal
 * @retval -1 no ready signal
 * @retval >0 ready signal number
 */
static inline int get_ready_signal(uint16_t periph_no){
    int res;
    // read lock here to make sure read consistency 
    mutex_read_lock(&sigpending_mutexes[periph_no]);
    // for test, record poll times
    poll_times[periph_no]++;
    if(io_helper_sigpending[periph_no]){
        int idx = __builtin_ffsll(io_helper_sigpending[periph_no]);
        // io_helper_sigpending &= ~(((uint64_t)1 << idx) - 1);
        // when data was taked, clear the bit
        res=idx-1;

        // for test, get ready signo, copy poll times to ready signo and reset poll times
        io_helper_signal_table[periph_no][res].poll_times=poll_times[periph_no];
        poll_times[periph_no]=0;
        
    }else{
        res=-1;
    }
    mutex_read_unlock(&sigpending_mutexes[periph_no]);
    return res;
}

/**
 * @brief set ready signal
 */
static inline void set_ready_signal(uint16_t periph_no,uint32_t signo){
    mutex_lock(&sigpending_mutexes[periph_no]);
    io_helper_sigpending[periph_no] |= ((uint64_t)1<<signo);
    mutex_unlock(&sigpending_mutexes[periph_no]);
}

/**
 * @brief check signal ready
 */
static inline int is_ready_signal(uint16_t periph_no,uint32_t signo){
    int res;
    mutex_read_lock(&sigpending_mutexes[periph_no]);
    res = (io_helper_sigpending[periph_no] & ((uint64_t)1 << signo)) != 0;
    mutex_read_unlock(&sigpending_mutexes[periph_no]);
    return res;
}

/**
 * @brief allocate avaliable signal number
 * @retval -1 no available signal number
 * @retval >0 available signal number
 */
static inline int allocate_signo(uint16_t periph_no){
    int res;
    mutex_lock(&sigusing_mutexes[periph_no]);
    // EMSG("sigusing: %016x",io_helper_sigusing[periph_no]);
    if(io_helper_sigusing[periph_no]==UINT64_MAX){
        res = -1;
    }else{
        int idx = __builtin_ffsll(~io_helper_sigusing[periph_no]);
        // fit corresponding bit
        if(idx==64){
            io_helper_sigusing[periph_no]=UINT64_MAX;
        }else{
            io_helper_sigusing[periph_no] |= (((uint64_t)1 << idx)-1);
        }
        req_nums[periph_no]++;
        res = idx-1;
        // for test
        io_helper_signal_table[periph_no][idx-1].poll_times=0;
    }
    mutex_unlock(&sigusing_mutexes[periph_no]);
    return res;
}

/**
 * @brief release using signo
 */
static inline void release_signo(uint16_t periph_no,uint32_t signo){
    // lock pending, then using
    mutex_lock(&sigpending_mutexes[periph_no]);
    mutex_lock(&sigusing_mutexes[periph_no]);
    io_helper_sigpending[periph_no] &= ~((uint64_t)1 << signo);
    io_helper_sigusing[periph_no] &= ~((uint64_t)1 << signo);
    req_nums[periph_no]--;
    mutex_unlock(&sigusing_mutexes[periph_no]);
    mutex_unlock(&sigpending_mutexes[periph_no]);
}

/**
 * @brief get req num
 */
static inline uint32_t get_req_num(uint16_t periph_no){
    uint32_t res;
    mutex_read_lock(&sigusing_mutexes[periph_no]);
    res=req_nums[periph_no];
    mutex_read_unlock(&sigusing_mutexes[periph_no]);
    return res;
}

static inline uint32_t is_using_signo(uint16_t periph_no, uint32_t signo){
    int res;
    mutex_read_lock(&sigusing_mutexes[periph_no]);
    res = (io_helper_sigusing[periph_no] & ((uint64_t)1 << signo)) != 0;
    mutex_read_unlock(&sigusing_mutexes[periph_no]);
    return res;
}

#endif
