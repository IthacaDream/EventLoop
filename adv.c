#include "adv.h"

/*
void daemonize() {
    int fd;
    
    if (fork() != 0) exit(0); // parent exits 
    setsid(); // create a new session 
    
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO) close(fd);
    }
}
*/

void write_log(int level, const char *fmt, ...) {
    const char *c = ".-*#";
    time_t now = time(NULL);
    va_list ap;
    FILE *fp;
    char buf[64];
    char msg[REDIS_MAX_LOGMSG_LEN];


    fp = (g_logfile == NULL) ? stdout : fopen(g_logfile,"a");
    if (!fp) return;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));
    fprintf(fp,"[%d] %s %c %s\n",(int)getpid(), buf, c[level], msg);
    fflush(fp);

    if (g_logfile) fclose(fp);
}

int main(int argc, char **argv) 
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    
    g_stat_numconnections = 0;
    g_logfile = strdup("./logfile.log");
    g_el = aeCreateEventLoop();

    int ipfd = anetTcpServer(g_neterr, SERVERPORT, NULL);

    if (ipfd == ANET_ERR) {
	write_log(LV_WARNING, "Opening port: %s", g_neterr);
	exit(1);
    }
    
    //aeCreateTimeEvent(g_el, 1, serverCron, NULL, NULL);
    if (aeCreateFileEvent(g_el, 
			  ipfd, 
			  AE_READABLE, 
			  acceptTcpHandler, 
			  NULL) == AE_ERR) {

	write_log(LV_WARNING, "%s: Out of memory\n","create event");
	sleep(1);
	abort();
    }
    
    aeMain(g_el);
    aeDeleteEventLoop(g_el);
    
    return 0;
}
