#ifndef __ADV_H__
#define __ADV_H__

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <assert.h>
#include <pthread.h>
#include <sys/uio.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/resource.h>


#include "ae.h"      /* Event driven programming library */
#include "anet.h"

#define REDIS_OK                0
#define REDIS_ERR               -1

/* Client request types */
#define REDIS_REQ_INLINE 1
#define REDIS_REQ_MULTIBULK 2


#define LV_DEBUG 0
#define LV_VERBOSE 1
#define LV_NOTICE 2
#define LV_WARNING 3

#define REDIS_BLOCKED 16    /* The client is waiting in a blocking operation */
#define REDIS_IO_WAIT 32    /* The client is waiting for Virtual Memory I/O */
#define REDIS_IOBUF_LEN         4092
#define REDIS_REPLY_CHUNK_BYTES (5*1500) /* 5 TCP packets with default MTU */

#define SERVERPORT        6379    /* TCP port */

#define REDIS_MAXIDLETIME       (60*5)  /* default client timeout */
#define REDIS_MAX_LOGMSG_LEN    1024 /* Default maximum length of syslog messages */
#define REDIS_CLOSE_AFTER_REPLY 128 /* Close after writing entire reply. */



// Clients are taken in a liked list. 
typedef struct redisClient 
{
    int fd;
    int flags;   
    //sds querybuf;
    char* querybuf;
    int sentlen;
    time_t lastinteraction; /* time of the last interaction, used for timeout */
} redisClient;


int  g_ipfd;
char *g_logfile;
uint64_t g_stat_numconnections;
char g_neterr[ANET_ERR_LEN];
aeEventLoop *g_el;


void write_log(int level, const char *fmt, ...);

void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);

#endif
