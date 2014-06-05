#include <stdio.h>


int main()
{
    char *errmsg = "ER:-6\n";
    int errcode = 0;
    if ( 1 != sscanf(errmsg, "ER:%i[^\n]", &errcode)){
        printf("wrong!");
    }
    printf("err code: %d\n", errcode);
    return 0;
}
