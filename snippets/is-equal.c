#include <stdio.h>

int main()
{
    int i = 0;
    if (2 == (i = 2)){
        printf("yeah! i = %d.\n", (i = 3) + (i = 4) );
    }
    return 0;
}
