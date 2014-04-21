#include <stdio.h>
#include <string.h>

int main()
{
    char *str;
    char *str1 = "hello, world\n";
    str = strdup(str1);
    printf("str: %s", str);
    return 0;
}
