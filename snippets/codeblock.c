#include <stdio.h>

int i = 10000;

int main()
{
    extern int i;
    int j = 10;
    int foo(){
        int i = 100;
        int fooo(){
            int i = 1000;
            return i;
        }
        return fooo();
    }

    if ( i == 10 ){
        int j = 100;
        printf("i: %d\n", i);
        printf("j: %d\n", j);
    } else {
        int j = 200;
        printf("i: %d\n", i);
        printf("j: %d\n", j);
    }

    printf("%d\n", foo());
    return 0;
}
