#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char *s = malloc(100);
    while(gets(s) != NULL){
        printf("s: %s\n", s);
    }
    printf("end!\n");
    return 0;
}
