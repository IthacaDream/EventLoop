#include "adv.h"
#include <sys/uio.h>


void freeClient(redisClient *c)
{
    //sdsfree(c->querybuf);
    free(c->querybuf);
    c->querybuf = NULL;
    
    aeDeleteFileEvent(g_el, c->fd, AE_READABLE);
    aeDeleteFileEvent(g_el, c->fd, AE_WRITABLE);
    close(c->fd);
    
    free(c);
}

void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    int nwritten = 0, totwritten = 0;
    redisClient *c = privdata;
    
    //totwritten = sdslen(c->querybuf);
    totwritten = strlen(c->querybuf);
    nwritten   = write(fd, c->querybuf+c->sentlen, totwritten-c->sentlen);
    if(nwritten == 0){
	//发送完毕
	freeClient(c);
	return ;
    }
    else if(nwritten < 0){
	if(errno == EAGAIN || errno == EWOULDBLOCK){
	    aeCreateFileEvent(g_el, c->fd, AE_WRITABLE, sendReplyToClient, c);
	    return ;
	}
	
	//异常退出
	write_log(LV_VERBOSE, "Writeing from client: %s", strerror(errno));
	freeClient(c);
	return ;
    }
    
    c->sentlen += nwritten;
    
    if(c->sentlen < totwritten){
	//没发送完接着再发
	aeCreateFileEvent(g_el, c->fd, AE_WRITABLE, sendReplyToClient, c);
	return ;
    }
 
    freeClient(c);
}

void processInputBuffer(redisClient *c) {
    c->querybuf = strcpy(c->querybuf, 
			 "HTTP/1.1 200 OK \r\nServer: nginx/1.0.5\r\nContent-Type: text/html\r\nContent-Length: 4\r\nDate: Sun, 14 Aug 2011 07:46:09 GMT\r\n\r\nqiye");
    
    // can write now
    aeCreateFileEvent(g_el, c->fd, AE_WRITABLE, sendReplyToClient, c);
}

void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) 
{
    redisClient *c = (redisClient*) privdata;
    char buf[REDIS_IOBUF_LEN];
    int nread;
	
    memset(buf, 0, REDIS_IOBUF_LEN);
    nread = read(fd, buf, REDIS_IOBUF_LEN);
    if (nread == -1) {
	if(errno == EAGAIN || errno == EWOULDBLOCK) {
	    //重新读取
	    aeCreateFileEvent(g_el, fd, AE_READABLE, readQueryFromClient, c);
	    return ;
	}
	else {
	    write_log(LV_VERBOSE, "Reading from client: %s", strerror(errno));
	    freeClient(c);
	    return;
	}
    } 
    else if (nread == 0) {
	write_log(LV_VERBOSE, "Client closed connection");
	freeClient(c);
	    return;
    }
    
    if (nread) {
        //c->querybuf = sdscatlen(c->querybuf, buf, nread);
	c->querybuf = strncat(c->querybuf, buf, nread);
        c->lastinteraction = time(NULL);
    }else{
	return;
    }
    
    
    if(strstr(c->querybuf, "\r\n\r\n")){
	processInputBuffer(c);  //yes, this is the process, need to do the right thing.
	return ;
    }
    
    //接着再读
    if (aeCreateFileEvent(g_el, fd, AE_READABLE, readQueryFromClient, c) == AE_ERR){
	close(fd);
	free(c);
	return ;
    }
    
}


redisClient *createClient(int fd){
    redisClient *c = malloc(sizeof(redisClient));
    
    anetNonBlock(NULL, fd);
    anetTcpNoDelay(NULL,fd);
    if (aeCreateFileEvent(g_el, fd, AE_READABLE, readQueryFromClient, c) == AE_ERR){
        close(fd);
        free(c);
        return NULL;
    }

    c->fd              = fd;
    //c->querybuf        = sdsempty();
    c->querybuf        = NULL;
    c->sentlen         = 0;
    c->lastinteraction = time(NULL);
    return c;
}

static void acceptCommonHandler(int fd) {
    redisClient *c;
    if ((c = createClient(fd)) == NULL) {
        write_log(LV_WARNING,"Error allocating resoures for the client");
        close(fd); // May be already closed, just ingore errors 
        return;
    }
    // If maxclient directive is set and this is one client more... close the
    // connection. Note that we create the client instead to check before
    // for this condition, since now the socket is already set in nonblocking
    // mode and we can send an error for free using the Kernel I/O 
    /*   if (g_maxclients && listLength(g_clients) > g_maxclients) 
	{
        char *err = "-ERR max number of clients reached\r\n";

        // That's a best effort error message, don't check write errors 
        if (write(c->fd,err,strlen(err)) == -1) 
		{
            // Nothing to do, Just to avoid the warning... 
        }
        freeClient(c);
        return;
    }*/
    g_stat_numconnections++;
}

void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cport, cfd;
    char cip[128];
    
    cfd = anetTcpAccept(g_neterr, fd, cip, &cport);
    if (cfd == AE_ERR) {
	write_log(LV_WARNING,"Accepting client connection: %s", g_neterr);
	return;
    }
    
    write_log(LV_VERBOSE,"Accepted %s:%d", cip, cport);
    acceptCommonHandler(cfd);
}
