#include <stdio.h>
#include <string.h>

int main()
{
    char buf[1024];
    bzero(buf, 1024);
    int len = strlen("abcdefg") + 1;
    printf("len: %d\n", len);
    strncpy(buf, "abcdefg", len);
    printf("len: %d\n", len);


    bzero(buf, 1024);
    fgets(buf, 1024, stdin);
    len = strlen(buf);
    printf("len: %d\n", len);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%d: %d\n", i, buf[i]);
    }

    return 0;
}
