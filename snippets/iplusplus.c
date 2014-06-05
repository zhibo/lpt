#include <stdio.h>


int main()
{
    int i = 10;
    int ii = 0;
    ii = i++;
    //i = i++ ;

    int j = 10;
    j = ++j;

    int k = 10;
    k = k + 1;

    printf("ii: %d, j: %d, k: %d\n", ii, j, k);
    return 0;
}
