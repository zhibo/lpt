#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define HOST  "google.com"

int main()
{
    char host[256];
    struct hostent *hptr;
    char **pptr;
    char tmp[256];
    strncpy(host, HOST, sizeof(HOST));


    int sockfd;
    struct addrinfo hints, *servinfo;
    struct in_addr addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;


    while(1){
        if (getaddrinfo(HOST, NULL, &hints, &servinfo) != 0) {
            printf("getaddrinfo: %s\n", strerror(errno));
            //return 0;
        } else {
            addr.s_addr = ((struct sockaddr_in *)(servinfo->ai_addr))->sin_addr.s_addr;
            printf("address: %s\n", inet_ntoa(addr));
            freeaddrinfo(servinfo);
        }

        if ((hptr = gethostbyname(host)) == NULL){
            fprintf(stderr, "gethotbyname error for host: %s: %s\n",
                    host, hstrerror(h_errno));
            //return 0;
        } else {
            if (hptr->h_addrtype == AF_INET 
                    && (pptr = hptr->h_addr_list) != NULL){
                inet_ntop(hptr->h_addrtype, *pptr, tmp, sizeof(tmp));
            }
            bzero(host, 256);
            strncpy(host, tmp, sizeof(tmp));
            printf("hostname: %s\n", host);
        }
        sleep(1);
    }
}
