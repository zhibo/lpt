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
#include <signal.h>

#define SERVER_ADDR     "192.168.1.1"
#define SERVER_PORT     8088
#define MAXLINE         4096
#define MAXSUB          4096 
#define PKT_SIZE        456


int fd = 0;

/* client type */
typedef enum _CLIENT_TYPE{
    UNKNOWN = 0,     /* undetermined */
    POST,            /* POST request */
    GET,             /* GET request */
} CLIENT_TYPE;


int sys_socket_init(int *fd_local)
{
    printf("relay_local_init\n");
	struct sockaddr_in addr;
    int ret, opt = 1;
    socklen_t len = sizeof(ret);

	*fd_local = socket(AF_INET, SOCK_STREAM, 0);
	if ( -1 == *fd_local ){
		return -1;
	}

    setsockopt(*fd_local, SOL_SOCKET, SO_KEEPALIVE, (void*)&opt, opt);
    setsockopt(*fd_local, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

    bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
/*     addr.sin_addr.s_addr = htonl(INADDR_ANY);
 */
    inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr);
	addr.sin_port = htons(SERVER_PORT);

    bind(*fd_local, (struct sockaddr*) &addr, sizeof(addr));
    listen(*fd_local, 5);

    return *fd_local;
}

int sys_recv_packet(int *fd, char *recvline)
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
    //printf("size = %d\n", size);
    return size;
}

int sys_send_packet(int *fd, const char *buf, int size)
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

int handle_post_read(int *fd, FILE *file)
{
    char recvline[MAXLINE + 1];
    int ret = 0;
    ret = sys_recv_packet(fd, recvline);
    recvline[ret] = '\0';
    //printf("%s", recvline);
    fprintf(file, "%s", recvline);
    return 0;
}

int handle_get_write(int *fd, FILE *file)
{
    int n = 0;
    char sendline[MAXLINE + 1];
    if ( !feof(file)) {
        fread(sendline, sizeof(char), PKT_SIZE, file);
        sys_send_packet(fd, sendline, strlen(sendline));
        sendline[PKT_SIZE] = '\0';
        //printf("%s", sendline);
    }
    return 0;
}


static CLIENT_TYPE _get_client_type(const char *recvline)
{
    if (0 == strncmp("POST /setaudio", recvline, sizeof("POST /setaudio") - 1)){
        printf("POST\n");
        return POST;
    } else if ( 0 == strncmp("GET /getaudio", 
                             recvline, sizeof("GET /getaudio") - 1)){
        printf("GET\n");
        return GET;
    } else {
        printf("UNKNOWN\n");
        return UNKNOWN;
    }
}

static CLIENT_TYPE get_client_type(int *fd)
{
    char recvline[MAXLINE + 1];
    int ret = 0;
    if ( ret = sys_recv_packet(fd, recvline) ){
        switch(ret){
            case 0:
                break;
            case 1:
                printf("block, wait...\n");
                break;
            case -1:
                printf("connection error.\n");
                return UNKNOWN;
                break;
            case -2:
                printf("no packet.\n");
                break;
            default:
                printf("ret size = %d\n", ret);
                break;
        }
    }
    return _get_client_type(recvline);
}

int http_send_404(int *fd)
{
    char sendline[MAXLINE + 1];
    snprintf(sendline, MAXSUB,
            "HTTP/1.0 404 Not Found\r\n"
            "Server: relay server/0.1\r\n"
            "Connent-Type: text/html\r\n"
            "Connection: keep-alive\r\n\r\n"
            "<html><title>404 Not Found</title>"
            "<body><center><h1>404 Not Found</h1></center>"
            "<p>The requested URL was not found.</p><hr>"
            "<center>relay server/0.1</center><hr></body></html>");
    if ( strlen(sendline) != sys_send_packet(fd, sendline, strlen(sendline)) ){
        printf("send finished with error.\n");
    }
    return 0;
}

int http_send_200(int *fd)
{
    char sendline[MAXLINE + 1];
    snprintf(sendline, MAXSUB,
            "HTTP/1.0 200 OK\r\n"
            "Server: relay server/0.1\r\n"
            "Connection: keep-alive\r\n\r\n");
    sys_send_packet(fd, sendline, strlen(sendline));
    return 0;
}

int http_send_ah(int *fd)
{
    char sendline[MAXLINE + 1];
    snprintf(sendline, MAXSUB,
            "HTTP/1.0 200 OK\r\n"
            "Server: relay server/0.1\r\n"
            "Content-Type: audio/x-wav\r\n"
            "Cache-Control: no-cache\r\n"
            "Pragma: no-cache\r\n"
            "Conntent-Length: 115200000\r\n"
            "Connection: close\r\n\r\n");
    sys_send_packet(fd, sendline, strlen(sendline));
    return 0;
}

int http_process_post(int *fd)
{
    fd_set fdr, fdw;
    struct timeval tv = {0, 0};
    int ret;
    printf("--------process post request.-------\n\n");
    FILE *file;
    file = fopen("/tmp/audio", "a");

    while(1){
	    FD_ZERO(&fdr);
        FD_ZERO(&fdw);
	    FD_SET(*fd, &fdr);
	    FD_SET(*fd, &fdw);
        ret = select(*fd+ 1, &fdr, &fdw, NULL, &tv);
	    if ( ret < 0 ) {
	    	printf("select return %d, errno: %s\n", ret, strerror(errno));
	    	break; /* socket timeout */
	    }

        if ( FD_ISSET(*fd, &fdr)) {
            ret = handle_post_read(fd, file);
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
    fclose(file);
	printf("relay server closed\n");
	return 0;
}

int http_process_get(int *fd)
{
    fd_set fdr, fdw;
    struct timeval tv = {0, 0};
    int ret;
    printf("--------process get request.-------\n\n");
    FILE *file;
    file = fopen("/tmp/audio", "r");

    while(1){
	    FD_ZERO(&fdr);
        FD_ZERO(&fdw);
	    FD_SET(*fd, &fdr);
	    FD_SET(*fd, &fdw);
        ret = select(*fd+ 1, &fdr, &fdw, NULL, &tv);
	    if ( ret < 0 ) {
	    	printf("select return %d, errno: %s\n", ret, strerror(errno));
	    	break; /* socket timeout */
	    }

        if ( FD_ISSET(*fd, &fdw)) {
            ret = handle_get_write(fd, file);
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
    fclose(file);
	printf("relay server closed\n");
	return 0;
}

int thread_client_worker(int *fd)
{
    fd_set fdr, fdw;
    struct timeval tv = {0, 0};
    int ret;
    printf("========relay server started.========\n\n");

    CLIENT_TYPE type;
    type = get_client_type(fd);
    printf("type: %d\n", type);
    switch(type){
        case UNKNOWN:
            printf("hello UNKNOWN\n");
            if ( 0 != http_send_404(fd)){
                printf("send 404 failed\n");
            }
            break;
        case POST:
            if ( 0 != http_send_200(fd)){
                printf("send 200 failed\n");
            }
            if ( 0 != http_process_post(fd)){
                printf("process post request failed.\n");
            }
            break;
        case GET:
            if ( 0 != http_send_ah(fd)){
                printf("send audio head failed\n");
            }
            if ( 0 != http_process_get(fd)){
                printf("process get request failed.\n");
            }
            break;
        default:
            break;
    }
    close(*fd);
}

static void _server_quit(int sig){
    if ( -1 != fd){
        close(fd);
    }
    printf("server closed\n");
    exit(0);
}

int main()
{
    int conn = 0;
    struct sockaddr_in cliaddr;
    char buff[50];
    int bufflen;
    pid_t pid;
    sys_socket_init(&fd);
    signal(SIGINT, _server_quit);
    printf("relay: waiting for connection...\n");
    bzero(&cliaddr, sizeof(cliaddr));
    while(1){
        conn = accept(fd, (struct sockaddr *)&cliaddr, &bufflen);
        if ( conn < 0  ){
            printf("relay: connection failed!, %s\n", strerror(errno));
            continue;
        }
        printf("server: conn = %d\n", conn);
        printf("client connection from %s, port %d\n", 
                inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
                ntohs(cliaddr.sin_port));
        if ( (pid == fork()) == 0 ) {
            thread_client_worker(&conn);
        } 
        close(conn);
    }
    close(fd);
    return 0;
}
