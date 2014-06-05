#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char *a = "hello, owlrd\n";
    char *b = malloc(100);
    memcpy(b, a, 0);
    return 0;
}
