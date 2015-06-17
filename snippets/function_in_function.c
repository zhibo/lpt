#include <stdio.h>

int foofoo(){
    int foo(){
        return 1;
    }
    return foo();
}

int main()
{
    printf("%d\n", foofoo());
    return 0;
}
