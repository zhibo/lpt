#include <stdio.h>
#include <string.h>


int main()
{
    char str[100] = "upgrade\nuri\ntoken\n";
    char *tmp, *uri, *token;

    uri = strtok_r(str, "\n", &tmp);
    token = strtok_r(NULL, "\n", &tmp);


    printf("uri: %s, token: %s\n", uri, token);
    printf("str: %s\n", str);
    return 0;
}
