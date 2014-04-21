#include <stdio.h>
#include <string.h>

int main()
{
    char a[10];
    memset(a, 0, 10);
    a[9] = '\0';

    char *b = "123456";
    printf("sizeof(int): %d\n", (int)sizeof(int));
    printf("sizeof(char): %d\n", (int)sizeof(char));
    printf("sizeof(long): %d\n", (int)sizeof(long));
    printf("sizeof(short): %d\n", (int)sizeof(short));
    printf("sizeof(char *): %d\n", (int)sizeof(char *));
    printf("sizeof(char[10]): %d\n", (int)sizeof(char[10]));
    printf("strlen(a[10]): %d\n", (int)strlen(a));
    printf("sizeof(a[10]): %d\n", (int)sizeof(a));
    printf("strlen(b): %d\n", (int)strlen(b));
    printf("sizeof(\"123456\"): %d\n", (int)sizeof("123456"));
    printf("sizeof(\"abc\"): %d\n", (int)sizeof("abc"));
/*     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 *     printf("sizeof(type): %d\n", (int)sizeof(type));
 */
    return 0;
}
