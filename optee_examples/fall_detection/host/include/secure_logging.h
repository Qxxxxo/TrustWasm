#ifndef _SECURE_LOGGING_H
#define _SECURE_LOGGING_H
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3 

void secure_log(int level,const char * func, int line, const char * format, ...);

#define LOG(level, format,...) secure_log(level,__FUNCTION__,__LINE__,format, ##__VA_ARGS__)
#define INFO(format,...) LOG(LOG_INFO,format,##__VA_ARGS__)
#define WARNING(format,...) LOG(LOG_WARNING,format,##__VA_ARGS__)
#define ERROR(format,...) LOG(LOG_ERROR,format,##__VA_ARGS__)
#endif