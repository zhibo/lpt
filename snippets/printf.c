#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("hello,world!\n");
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        sleep(1);
        fputs("\ri = %d\n", i, stdout);
    }
    return 0;
}
