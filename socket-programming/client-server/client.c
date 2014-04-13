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
#define LOCAL_IP    "192.168.1.167"
#define REMOTE_IP   "192.168.1.1"
#define LOCAL_PORT  8080
#define REMOTE_PORT 8080
#define MAXSUB      200

static int _socket_init( int *fd, char *host, int port)
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


ssize_t _tcp_send(int *fd_to, const char *buf, int size)
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


void _str_cli(FILE *fp, int fd)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    while(fgets(sendline, MAXLINE, fp) != NULL){
        send(fd, sendline, strlen(sendline), 0);
        if (recv(fd, recvline, MAXLINE, 0) == 0 ){
            printf("server terminated.\n");
        }
        fputs(recvline, stdout);
    }
}

int main()
{
    int fd = 0;
    _socket_init(&fd, REMOTE_IP, REMOTE_PORT);
    printf("%d\n", fd);
    _str_cli(stdin, fd);
}
