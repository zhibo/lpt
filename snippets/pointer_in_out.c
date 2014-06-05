#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void foo(char *p)
{
    strncpy(p, "hello", 10);
}

int main()
{
    char *p = malloc(10);
    foo(p);
    printf("p: %s", p);
}
