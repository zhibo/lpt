#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func(int i)
{
    i = 2;
    printf("(in)i = %d\n", i);
}

void foo( char *p )
{
    printf("(in)address of p is: %p\n", p);
    //p[1] = 'd';
    strcpy(p, "cba");
    ++p;
    printf("(in)address of p is: %p\n", p);
    printf("(in)p is %s\n", p);
}


int main()
{

    if(-1){
        printf("-1 is ture\n");
    }
    int i = 1;
    printf("(pre)i = %d\n", i);
    func(i);
    printf("(post)i = %d\n", i);

    char *p;
    p = (char *)malloc(sizeof(char) * 8);
    strcpy(p, "abc");
    printf("(pre)address of p is: %p\n", p);
    printf("(pre)p is %s\n", p);
    foo(p);
    printf("(post)address of p is: %p\n", p);
    printf("(post)p is %s\n", p);
    return 0;
}
