#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i;
    char buffer[256];

    printf("Enter a number: ");
    fgets(buffer, 256, stdin);

    i = atoi(buffer);
    printf("The Value entered is %d\n", i);
    return 0;
}
