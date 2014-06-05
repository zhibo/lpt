#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


void* thread_function(void* arg)
{
    pthread_detach(pthread_self());
    int i = 0;
    for(i = 0; i < 15; i++){
        sleep(1);
        printf("i am new thread.");
    }
    sleep(500);
}

int main()
{
    pthread_t thread;
    
    pthread_create(&thread, NULL, thread_function, NULL);

    printf("hello, i am main.\n");

    sleep(1000);
    return 0;
}
