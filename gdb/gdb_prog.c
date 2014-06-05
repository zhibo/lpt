#include <stdio.h>
#include <string.h>
#include <strings.h>

int main()
{
    char str[256] = "hello, world!";
    bzero(str, 512);
    strncpy(str, "ok", 256);
    printf("str: %s", str);
    return 0;
}
