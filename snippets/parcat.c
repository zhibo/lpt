#include <stdio.h>

#define STR(s) #s
#define CONS(a, b) (int)(a##e##b)

int main()
{
    printf(STR(abcdefg));
    printf("\n");
    printf("%d\n", CONS(2,3));
    printf("%f\n", 2e3);
    return 0;
}
