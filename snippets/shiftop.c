#include <stdio.h>


int main()
{
    int i = -7;
    printf("i: %x, i: %d\n", i, i);
    i >>= 1;
    printf("i: %x, i: %d\n", i, i);
    i <<= 1;
    printf("i: %x, i: %d\n", i, i);
    i <<= 1;
    printf("i: %x, i: %d\n", i, i);
    return 0;
}
