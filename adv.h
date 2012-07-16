#ifndef __ADV_H
#define __ADV_H

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


#include "ae.h"     /* Event driven programming library */
#include "sds.h"    /* Dynamic safe strings */
#include "zmalloc.h" /* total memory usage aware version of malloc/free */
#include "anet.h"

#define REDIS_OK                0
#define REDIS_ERR               -1

/* Client request types */
#define REDIS_REQ_INLINE 1
#define REDIS_REQ_MULTIBULK 2


#define REDIS_DEBUG 0
#define REDIS_VERBOSE 1
#define REDIS_NOTICE 2
#define REDIS_WARNING 3

#define REDIS_BLOCKED 16    /* The client is waiting in a blocking operation */
#define REDIS_IO_WAIT 32    /* The client is waiting for Virtual Memory I/O */
#define REDIS_IOBUF_LEN         4092
#define REDIS_REPLY_CHUNK_BYTES (5*1500) /* 5 TCP packets with default MTU */
#define REDIS_SERVERPORT        6379    /* TCP port */
#define REDIS_MAXIDLETIME       (60*5)  /* default client timeout */
#define REDIS_MAX_LOGMSG_LEN    1024 /* Default maximum length of syslog messages */
#define REDIS_CLOSE_AFTER_REPLY 128 /* Close after writing entire reply. */
#define REDIS_NOTUSED(V) ((void) V)


/* We can print the stacktrace, so our assert is defined this way: */
#define redisAssert(_e) ((_e)?(void)0 : (_redisAssert(#_e,__FILE__,__LINE__),_exit(1)))
#define redisPanic(_e) _redisPanic(#_e,__FILE__,__LINE__),_exit(1)
void _redisAssert(char *estr, char *file, int line);
void _redisPanic(char *msg, char *file, int line);


// Clients are taken in a liked list. 
typedef struct redisClient 
{
    int fd;
	int flags;   
    sds querybuf;
    int sentlen;
    time_t lastinteraction; /* time of the last interaction, used for timeout */
} redisClient;

struct redisServer
{
	int  ipfd;
	int  port;
	char *logfile;
	char *bindaddr;
	
	uint64_t stat_numconnections;
	char neterr[ANET_ERR_LEN];
	aeEventLoop *el;
	int daemonize;
};


struct redisServer server;

void redisLog(int level, const char *fmt, ...) ;

void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);

#endif
