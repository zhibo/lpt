#include <stdio.h>

struct s1{
    char aa;
    char bb;
    char cc;
    char a;
    char c;
    int b;
};

int main()
{
    struct s1 s;
    printf("%d\n", (int)sizeof(s));

    return 0;
}
