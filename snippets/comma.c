#include <stdio.h>

int func(int c)
{
    printf("hello,world: %d\n", c);
    return c;
}

int main()
{
    int a = 1, b = 2, c = 3;
    a = ( b, func(c) );
    printf("a = %d\n", a);
    return 0;
}
