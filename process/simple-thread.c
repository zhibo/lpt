#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread(void *arg)
{
    fprintf(stderr, "thread id is %d\n", getpid());
    sleep(100);
    return NULL;
}

int main()
{
    pthread_t *th;

    printf("main process id is %d\n", getpid());
    if ( 0 != pthread_create(th, NULL, thread, NULL) ){
        perror("test pthread_create\n");
    }

    sleep(100);
    return 0;
}
