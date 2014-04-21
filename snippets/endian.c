#include <stdio.h>

int main()
{
    union {
        short s;
        char c[sizeof(short)];
    } un;


    un.s = 0x0102;
    printf("s @ %p, and s = %d\n", &un.s, un.s);
    printf("c[0] @ %p, and c[0] = %d\n", &un.c[0], un.c[0]);
    printf("c[1] @ %p, and c[1] = %d\n", &un.c[1], un.c[1]);
    if ( sizeof(short) == 2 ){
        if (un.c[0] == 1 && un.c[1] == 2)
            printf("big-endian\n");
        else if (un.c[0] == 2 && un.c[1] == 1)
            printf("little-endian\n");
        else
            printf("unknown\n");
    } else
        printf("sizeof(short) = %d\n", (int)sizeof(short));
    return 0;
}
