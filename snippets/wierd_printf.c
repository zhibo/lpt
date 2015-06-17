#include <stdio.h>
#include <string.h>

int main()
{
    char buf[1024] = {0};
    char *tmp;

    sprintf(buf, "hello, world!\n");
    tmp = buf + (int)strlen(buf) - 1;
    *tmp = 0;

    printf("request: %s, tmp: %s\n", buf, tmp);
    return 0;
}
