#include <stdio.h>

int main()
{
    char arr[10] = "hello";
    char *p = arr;      // p is a pointer to char
    const char *q = arr;    // q is a pointer to const char
    char const *qq = arr;  // qq is a pointer to const char
    char * const r = arr;   // r is a const pointer to char
    const char * const s = arr;  // s is a const pointer to const char
    return 0;
}
