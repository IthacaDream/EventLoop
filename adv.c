#include "adv.h"


void daemonize() 
{
    int fd;

    if (fork() != 0) exit(0); /* parent exits */
    setsid(); /* create a new session */

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) 
	{
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
}

void oom(const char *msg) 
{
    redisLog(REDIS_WARNING, "%s: Out of memory\n",msg);
    sleep(1);
    abort();
}

void redisLog(int level, const char *fmt, ...) 
{
    const char *c = ".-*#";
    time_t now = time(NULL);
    va_list ap;
    FILE *fp;
    char buf[64];
    char msg[REDIS_MAX_LOGMSG_LEN];


    fp = (server.logfile == NULL) ? stdout : fopen(server.logfile,"a");
    if (!fp) return;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));
    fprintf(fp,"[%d] %s %c %s\n",(int)getpid(), buf, c[level], msg);
    fflush(fp);

    if (server.logfile) fclose(fp);
}

int main(int argc, char **argv) 
{
	signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
	
	server.stat_numconnections = 0;
	server.logfile  = zstrdup("./redis.log");
	server.bindaddr = NULL;
	server.port     = REDIS_SERVERPORT;
	server.el       = aeCreateEventLoop();
	server.ipfd     = anetTcpServer(server.neterr, server.port, server.bindaddr);
	if (server.ipfd == ANET_ERR) 
	{
		redisLog(REDIS_WARNING, "Opening port: %s", server.neterr);
		exit(1);
	}
	
	//aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL);
    if (aeCreateFileEvent(server.el, server.ipfd, AE_READABLE, acceptTcpHandler, NULL) == AE_ERR) 
		oom("creating file event");
	aeMain(server.el);
	aeDeleteEventLoop(server.el);
	
	return 0;
}

