#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define LOCAL_PORT      10800
#define MAXLINE         4096
#define MAXSUB          200 
#define PKT_SIZE        456

/* client type */
typedef enum _CLIENT_TYPE{
    UNKNOWN = 0,     /* undetermined */
    POST,            /* POST request */
    GET,             /* GET request */
} CLIENT_TYPE;


static int _socket_init(int *fd)
{
	struct sockaddr_in addr;
    int ret;
    socklen_t len = sizeof(ret);

	*fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( -1 == *fd ){
		return -1;
	}

    setsockopt(*fd, SOL_SOCKET, SO_KEEPALIVE, &len, len);

    bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(LOCAL_PORT);

    bind(*fd, (struct sockaddr*) &addr, sizeof(addr));
    listen(*fd, 5);

    return *fd;
}

static int _tcp_read(int *fd, char *recvline)
{
    int size;
    size = recv(*fd, recvline, MAXLINE, 0);
	if ( -1 == size && (EAGAIN == errno || EWOULDBLOCK == errno)) 
	{
		printf("tcp recv would block\n");
		return 1; /* indicate operation would block */
	} 
	if ( size < 0 ) {
		printf("tcp recv return %d bytes, errno %d\n", size, errno);
		return -1;
	}
    if ( size == 0 ){
        return -2;  /* no packets */
    }
    return size;
}

static int _tcp_send(int *fd, const char *buf, int size)
{
    int sent, off = 0;
	do {
		sent = send(*fd, buf+off, size-off, 0);
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
    return off;
}

static int _str_echo(int fd)
{
    ssize_t n;
    char buf[MAXLINE + 1];

again:
    while ((n = read(fd, buf, MAXLINE)) > 0){
        if ( send(fd, buf, n, 0) < 0){
            printf("server send error, %s\n", strerror(errno));
        }
    }
    if ( n < 0 && errno == EINTR){
        goto again;
    } else if ( n < 0){
        printf("server read error, %s\n", strerror(errno));
    }
}

int main()
{
    int fd, conn = 0;
    struct sockaddr_in cliaddr;
    char buff[50];
    int bufflen;
    pid_t pid;
    _socket_init(&fd);
    bzero(&cliaddr, sizeof(cliaddr));
    while(1){
        conn = accept(fd, (struct sockaddr *)&cliaddr, &bufflen);
        if ( conn < 0  ){
            printf("server: connection failed!, %s\n", strerror(errno));
            continue;
        }
        printf("conn = %d\n", conn);
        printf("client connection from %s, port %d\n", 
                inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
                ntohs(cliaddr.sin_port));
        if ( (pid == fork()) == 0 ) {
            _str_echo(conn);
        }
        close(conn);
    }
    close(fd);
    return 0;
}
