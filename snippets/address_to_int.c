#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a = 18;
    char result[128];
    snprintf(result, 128, "%u", (unsigned int)&a);
    printf("result: %s\n", result);
    unsigned int p = (unsigned int)atoi(result);
    printf("p:%u\n", p);
    unsigned long q = (unsigned long)p;
    printf("q:%lu\n", q);
    printf("sizeof(char*): %ld\n", sizeof(char*));
    printf("sizeof(unsigned int): %ld\n", sizeof(unsigned int));
    printf("sizeof(int): %ld\n", sizeof(int));
    printf("sizeof(unsigned long): %ld\n", sizeof(unsigned long));
    printf("p:hex:%p, dec:%lu\n", (char *)q, q);
    printf("a: %d\n", *((char*)&a));
    printf("result: %d\n", *((int*)q));
    return 0;
}
