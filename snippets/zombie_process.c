#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    pid_t pid, ppid;
    printf("Hello World1\n");
    pid=fork();
    if(pid==0)
    {
    }
    else
    {
        while(1)
        {
        printf("I am the parent\n");
        printf("The PID of parent is %d\n",getpid());
        printf("The PID of parent of parent is %d\n",getppid());        
        sleep(2);
        }
    }
}
