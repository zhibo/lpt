#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAXLINE     4096
#define LOCAL_IP    "192.168.1.177"
#define REMOTE_IP   "192.168.1.1"
#define LOCAL_PORT  8080
#define REMOTE_PORT 8088
#define MAXSUB      200

int socket_init( int *fd, char *host, int port)
{
    struct sockaddr_in addr;
    int ret;
    socklen_t len = sizeof(ret);
    if ( !fd ){
        return -1;
    }

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(*fd, SOL_SOCKET, SO_KEEPALIVE, &len, len);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    connect(*fd, (struct sockaddr *)&addr, sizeof(addr));
}


ssize_t tcp_send(int *fd_to, const char *buf, int size)
{
    int sent, off = 0;
	do {
		sent = send(*fd_to, buf+off, size-off, 0);
		if ( -1 == sent && (EAGAIN == errno || EWOULDBLOCK == errno) )
		{
            usleep(20 * 1000);
			continue;
		}
		if ( sent < 0 ) {
			printf("tcp send return -1, errno %d\n", errno);
			return -1;
		}
		off += sent;
	} while( off < size );
    return 0;
}

ssize_t http_get(int *fd)
{
    char sendline[MAXLINE + 1];
    snprintf(sendline, MAXSUB, 
            "GET /stream/getaudio HTTP/1.0\r\n"
            "Connection: Keep-Alive\r\n"
            "Authorization: Basic YWRtaW46YWRtaW4=\r\n"
            "Accept: */*\r\n\r\n",
            LOCAL_IP);

    send(*fd, sendline, strlen(sendline), 0);
}

ssize_t process_local(int *fd_local, int *fd_remote )
{
    char recvline[MAXLINE + 1], headline[MAXLINE + 1];
    int size, len, ret;
    char * h;
    size = recv(*fd_local, recvline, MAXLINE, 0);
	if ( -1 == size && (EAGAIN == errno || EWOULDBLOCK == errno)) 
	{
		printf("tcp recv would block\n");
		return 1; /* indicate operation would block */
	} 
	if ( size <= 0 ) {
		printf("tcp recv return %d bytes, errno %d\n", size, errno);
		return -1;
	}
    if (0 == strncmp("HTTP/1.0", recvline, sizeof("HTTP/1.0") - 1)){
        snprintf(headline, MAXSUB, 
                "POST /setaudio HTTP/1.0\r\n"
                "Host: %s:8080\r\n"
                "Content-Type: audio/x-wav\r\n"
                "Content-length: 115200000\r\n\r\n"
                "RIFF2WAVEfmt ", REMOTE_IP
                );
        printf("%s", headline);
        ret = tcp_send(fd_remote, headline, strlen(headline));
    } else {
        printf("%s", recvline);
        ret = tcp_send(fd_remote, recvline, strlen(recvline));
    }
    return ret;
}


ssize_t process_remote(int *fd_remote)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    int size;
    size = recv(*fd_remote, recvline, MAXLINE, 0);
	if ( -1 == size && (EAGAIN == errno || EWOULDBLOCK == errno)) 
	{
		printf("tcp recv would block\n");
		return 1; /* indicate operation would block */
	} 
	if ( size <= 0 ) {
		printf("tcp recv return %d bytes, errno %d\n", size, errno);
		return -1;
	}
    recvline[size] = '\0';
    printf("%s", recvline);
    return 0;
}

int process_relay(int *fd_local, int *fd_remote)
{
    fd_set fdr, fdw;
    struct timeval tv = {0, 0};
    int ret;
    printf("start relay\n\n\n\n\n");

    while(1){
	    FD_ZERO(&fdr);
	    FD_SET(*fd_local, &fdr);
	    FD_SET(*fd_remote, &fdr);
        ret = select(*fd_local > *fd_remote ? *fd_local + 1 : *fd_remote + 1,
                &fdr, NULL, NULL, &tv);
	    if ( ret < 0 ) {
	    	printf("select return %d, errno %d\n", ret, errno);
	    	break; /* socket timeout */
	    }

	    FD_ZERO(&fdw);
	    FD_SET(*fd_local, &fdw);
	    FD_SET(*fd_remote, &fdw);
        ret = select(*fd_local > *fd_remote ? *fd_local + 1 : *fd_remote + 1, 
                NULL, &fdw, NULL, &tv);
	    if ( ret < 0 ) {
	    	printf("select return %d, errno %d\n", ret, errno);
	    	break; /* socket timeout */
	    }

        if ( FD_ISSET(*fd_local, &fdr) && FD_ISSET(*fd_remote, &fdw)) {
        //if ( FD_ISSET(*fd_local, &fdr)){
            ret = process_local(fd_local, fd_remote);
        }

        if ( FD_ISSET(*fd_remote, &fdr)) {
            ret = process_remote(fd_remote);
        }


		if ( ret > 0 ) {
		   	/* sockets have events, but operation would block */
			usleep(20 * 1000);
		}
		if ( -1 == ret ) {
			/* socket error */
			printf("relay: socket error\n");
			break;
		}
    }
	printf("relay: relay finished\n");
	close(*fd_local);
	close(*fd_remote);

	return 0;
}

int main()
{
    int lfd, rfd = 0;
    socket_init(&lfd, LOCAL_IP, LOCAL_PORT);
    socket_init(&rfd, REMOTE_IP, REMOTE_PORT);
    http_get(&lfd);
    process_relay(&lfd, &rfd);
}
