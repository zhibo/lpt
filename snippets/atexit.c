#include <stdio.h>
#include <stdlib.h>

void f()
{
    printf("This is f\n");
}

void g()
{
    printf("This is g\n");
}

int main()
{
    atexit(f);
    atexit(g);
    printf("This is main\n");
    return 0;
}
