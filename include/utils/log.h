#ifndef  DEBUG_LOG_H
#define  DEBUG_LOG_H

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum LOG_LEVEL_E
{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_MAX,
}LOG_LEVEL;

/* :if you want output the log msg to file or net,just modify __LOG_PRINT__ as a function */
#define __LOG_PRINT__(level,module,...)\
    do{ \
        if ( LOG_LEVEL_INFO == level ) \
        { \
            printf("INFO\t");\
        } \
	else if ( LOG_LEVEL_WARNING == level )	\
        { \
            printf("WARNING\t");\
        } \
	else if ( LOG_LEVEL_DEBUG == level )	\
        { \
            printf("DEBUG\t");\
        } \
	else if ( LOG_LEVEL_ERROR == level )	\
        { \
            printf("ERROR\t");\
        } \
	printf(module" function:%s line:%d > ",__FUNCTION__,__LINE__);	\
	printf(__VA_ARGS__);\
	printf("\r\n");\
    }while(0)

    
#ifndef LOGV
#define LOGV(...) __LOG_PRINT__(LOG_LEVEL_INFO, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(...) __LOG_PRINT__(LOG_LEVEL_WARNING, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGD
#define LOGD(...) __LOG_PRINT__(LOG_LEVEL_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...) __LOG_PRINT__(LOG_LEVEL_ERROR, LOG_TAG, __VA_ARGS__)
#endif


#ifdef __cplusplus
}
#endif

#endif   //DEBUG_LOG_H


