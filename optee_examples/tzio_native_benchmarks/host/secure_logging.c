#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include "secure_logging.h"

static pthread_mutex_t log_mutex;

static const char * get_current_time(){
    static char time_str[20];
    time_t t = time(NULL);
    struct tm * time_info = localtime(&t);
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",time_info);
    return time_str;
}

void secure_log(int level,const char * func, int line, const char * format, ...){
    const char * level_str; 
    switch (level){
        case LOG_INFO:
            level_str="INFO";
            break;
        case LOG_WARNING:
            level_str="WARNING";
            break;
        case LOG_ERROR:
            level_str="ERROR";
            break;
        default:
            level_str="UNKNOWN";
            break;
    }

    va_list args;
    va_start(args,format);
    pthread_mutex_lock(&log_mutex);
    printf("[%s] [%s] [%s:%d] ",get_current_time(),level_str,func,line);
    vprintf(format,args);
    printf("\n");
    va_end(args);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}