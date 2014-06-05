#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct test {
    char *p;
    int i;
} t;

char *foo(struct test *t)
{
    return t->p;
}

char *foo2()
{
    char *p;
    //p = (char*)malloc(10);
    printf("P:%p\n", p);
    return p;
}


int main()
{
    t.p = strdup("hello, world!");
    printf("%p\n", t.p);
    printf("%p\n", foo(&t));
    printf("%s\n", foo(&t));
    char *xxx = foo2();
    printf("X:%p\n", xxx);
    return 0;
}
