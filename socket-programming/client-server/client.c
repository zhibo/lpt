#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
//#include <netinet/tcp.h>
#include <linux/tcp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define MAXLINE     4096
#define LOCAL_IP    "192.168.1.1"
#define REMOTE_IP   "172.29.86.5"
#define REMOTE_PORT 10800
#define MAXSUB      200

int fd = 0;

static int _set_interface(int fd, const char* interface_name)
{
    struct ifreq interface;
    memset(&interface, 0, sizeof(interface));
    strncpy(interface.ifr_name, interface_name, IFNAMSIZ);
    int res = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface));
    return res;
}

static int _socket_init( int *fd, char *host, int port)
{
    struct sockaddr_in addr, local_addr;
    int ret;
    int keep_alive = 1;
    int keep_idle = 5, keep_interval = 2, keep_count = 3;
    int timeout = 10000;
    socklen_t len = sizeof(ret);
    if ( !fd ){
        return -1;
    }

    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( -1 == setsockopt(*fd, SOL_SOCKET, SO_KEEPALIVE, 
                &keep_alive, sizeof(keep_alive))){
        printf("SO_KEEPALVIE set failed, %s\n", strerror(errno));
    }
    if ( -1 == setsockopt(*fd, IPPROTO_TCP, TCP_KEEPIDLE, 
                &keep_idle, sizeof(keep_idle))){
        printf("TCP_KEEPIDLE set failed, %s\n", strerror(errno));
    }
    if ( -1 == setsockopt(*fd, IPPROTO_TCP, TCP_KEEPINTVL, 
                &keep_interval, sizeof(keep_interval))){
        printf("TCP_KEEPINTVL set failed, %s\n", strerror(errno));
    }
    if ( -1 == setsockopt(*fd, IPPROTO_TCP, TCP_KEEPCNT, 
                &keep_count, sizeof(keep_count))){
        printf("TCP_KEEPCNT set failed, %s\n", strerror(errno));
    }
/*     if ( -1 == setsockopt(*fd, IPPROTO_TCP, TCP_USER_TIMEOUT, 
 *                 &timeout, sizeof(timeout))){
 *         printf("TCP_USER_TIMEOUT set failed, %s\n", strerror(errno));
 *     }
 */
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

/*     bzero(&local_addr, sizeof(local_addr));
 *     addr.sin_family = AF_INET;
 *     addr.sin_port = 0;
 *     inet_pton(AF_INET, "172.29.86.189", &addr.sin_addr);
 */


/*     if (bind(*fd, (struct sockaddr*) &local_addr, sizeof(local_addr)) == -1 ){
 *         printf("bind failed, %s\n", strerror(errno));
 *     }
 */
    _set_interface(*fd, "eth1");
    connect(*fd, (struct sockaddr *)&addr, sizeof(addr));
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
		printf("tcp recv return %d bytes, %s\n", size, strerror(errno));
		return -1;
	}
    if ( size == 0 ){
		printf("no packets, size: %d, %s\n", size, strerror(errno));
        return -2;  /* no packets */
    }
    return size;
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

static int _recv_echo(char recvline[], int fd)
{
    int ret, ec = 0;
    struct timeval tv = {0, 0};
    fd_set fdr;

    tv.tv_sec = 500;
    tv.tv_usec = 500;

    if ( -1 == fd ){
        sleep(tv.tv_sec);
        usleep(tv.tv_usec);
        return 0;
    }
    FD_ZERO(&fdr);
    FD_SET(fd, &fdr);

    ret = select(fd + 1, &fdr, NULL, NULL, &tv);
    
    ec = errno;
    printf("ret = %d, errno = %d, %s\n", ret, ec, strerror(ec));
    if ( 0 == ret || (-1 == ret && EINTR == errno)){
        //printf("ret = %d, errno = %d, %s\n", ret, errno, strerror(errno));
        return 0;
    }
    return 1;
}


static void _str_cli(FILE *fp, int fd)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    while(fgets(sendline, MAXLINE, fp) != NULL){
        errno = 0;
        if ( send(fd, sendline, strlen(sendline), MSG_NOSIGNAL) < 0){
            printf("client error, %s\n", strerror(errno));
        }

        bzero(recvline, strlen(recvline));
        if ( _recv_echo(recvline, fd) ){
            if ( recv(fd, recvline, MAXLINE, 0) <= 0 ){
                printf("server error, %s\n", strerror(errno));
            }
        }
        //_tcp_read(&fd, recvline);
        fputs(recvline, stdout);
        bzero(sendline, strlen(sendline));
        sleep(1);
    }
}

static void _client_quit(int sig){
    if ( -1 != fd){
        close(fd);
    }
    exit(0);
}

int main()
{
    signal(SIGINT, _client_quit);
    _socket_init(&fd, REMOTE_IP, REMOTE_PORT);
    printf("%d\n", fd);
    _str_cli(stdin, fd);
    return 0;
}
