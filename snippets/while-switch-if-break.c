#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    int state = 0;
    while(1)
    {
        switch(state)
        {
            case 0:
                printf("i am 0\n");
                sleep(1);
            case 1:
                printf("i am 1\n");
                break;
            default:
                printf("i am default\n");
                break;
        }
        sleep(1);
        printf("after switch, errno: %d\n", errno);
    }
    return 0;
}
