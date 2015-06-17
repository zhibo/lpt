#include <stdio.h>
#include <string.h>
#include <stdint.h>


void func(char *a)
{
    printf("sizeof(a) from parameter: %d\n", (int)sizeof(a));
}

int main()
{
    char a[10];
    memset(a, 0, 10);
    a[9] = '\0';

    char *b = "123456";
    printf("sizeof('c'): %d\n", (int)sizeof('c'));
    printf("sizeof(char): %d\n", (int)sizeof(char));
    printf("sizeof(uint32_t): %d\n", (int)sizeof(uint32_t));
    printf("sizeof(int): %d\n", (int)sizeof(int));
    printf("sizeof(char): %d\n", (int)sizeof(char));
    printf("sizeof(unsigned long): %d\n", (int)sizeof(unsigned long));
    printf("sizeof(short): %d\n", (int)sizeof(short));
    printf("sizeof(char *): %d\n", (int)sizeof(char *));
    printf("sizeof(char[10]): %d\n", (int)sizeof(char[10]));
    printf("strlen(a[10]): %d\n", (int)strlen(a));
    printf("sizeof(a[10]): %d\n", (int)sizeof(a));
    printf("strlen(b): %d\n", (int)strlen(b));
    printf("sizeof(\"123456\"): %d\n", (int)sizeof("123456"));
    printf("sizeof(\"abc\"): %d\n", (int)sizeof("abc"));
    {
        int i = 10;
        int j = 20;
        printf("sizeof(i=j+1): %d\n", (int)sizeof(i=j+1));
        printf("i: %d, j: %d\n", i, j);
        printf("sizeof(int): %d\n", (int)sizeof(int));
        printf("sizeof(i): %d\n", (int)sizeof(i));
    }

    func(a);
/*     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 */
    return 0;
}
