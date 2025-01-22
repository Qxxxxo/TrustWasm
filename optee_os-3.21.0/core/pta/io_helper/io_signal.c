/**
 * @brief io signal
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include "io_helper/io_signal.h"
io_sigentry_t io_helper_signal_table[N_PERIPH][N_SUB_SIG]={0};

uint64_t io_helper_sigpending[N_PERIPH]={0};
uint32_t poll_times[N_PERIPH]={0};
struct mutex sigpending_mutexes[N_PERIPH];

// init as all sigal avaliable and so signal ready
uint64_t io_helper_sigusing[N_PERIPH]={0};
uint32_t req_nums[N_PERIPH]={0};
struct mutex sigusing_mutexes[N_PERIPH];
