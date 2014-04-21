#include <stdio.h>
#include <string.h> 


int main()
{
    char *c = "help!";
    char cc[100] = "";
    snprintf(cc, 100, "%s", c);
    printf("cc: %s", cc);
    return 0;
}
