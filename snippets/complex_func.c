#include <stdio.h>


// char foo[10](void);  //wrong

char (*f[10])(char (*[10])(void));

char *(f10)(char *(*[10])(void));


char (*f1[10])(void);

char *array[10];

char foo(char *[10]);
