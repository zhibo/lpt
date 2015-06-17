#include <stdio.h>

void func(char a[20])
{
    printf("sizeof(a) = %d\n", (int)sizeof(a));
    printf("%s", a);
}

int main()
{
    char a[20] = "hello, world\n";
    func(a);
    return 0;
}
