#include <stdio.h>
#include <string.h>

int main()
{
    char str[20] = "HTTP 1.0 200 OK";
    if ( !strncmp("HTTP 1.0", str, sizeof("HTTP 1.0") - 1) ){
        printf("found!\n");
    }
    return 0;
}
