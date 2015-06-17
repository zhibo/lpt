#include <stdio.h>
#include <string.h>

struct test {
    int x;
    char str[4];
    void *p;
};


int main()
{
    struct test s;
    memset(&s, 0, sizeof(s));
    s.x = 10;
    s.str[0] = 'a';
    s.p = (void*)&s;


    int i = 0, len = sizeof(s);

    for ( ; i < len; ++i ){
        printf("0x%02x ", ((char*)&s)[i]);
    }
    printf("\n");
    return 0;
}
