#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void * test_thread(void * arg)
{
    //char *str = (char *)arg;
    char str[10];
    printf("arg: %p\n", arg);
    strncpy(str, (char*)arg, 10);
    pthread_detach(pthread_self());
    printf("str: %s\n\n", str);
    fflush(stdout);
    return NULL;
}

int main()
{
    int i = 0;
    for ( i = 0; i < 10; i++ ){
        char str[10];   /* not safe */
        printf("in str: %p\n", str);
        printf("sizeof: %d\n", (int)sizeof("hello"));
        strncpy(str, "hello", sizeof("hello") + 1);
        pthread_t thr;
        if ( pthread_create(&thr, NULL, test_thread, (void*)str)){
            return -1;
        }
        printf(".\n");
    }
    return 0;
}
