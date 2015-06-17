#include <stdio.h>

int is_valid_ip(const char* p)
{
    int n[4];
    char c[4];
    if (sscanf(p, "%d%c%d%c%d%c%d%c",
                &n[0], &c[0], &n[1], &c[1],
                &n[2], &c[2], &n[3], &c[3])
            == 7)
    {
        int i;
        for(i = 0; i < 3; ++i){
            printf("c[%d]: %c\n", i, c[i]);
            if (c[i] != '.')
                return 0;
        }
        for(i = 0; i < 4; ++i){
            printf("n[%d]: %d\n", i, n[i]);
            if (n[i] > 255 || n[i] < 0)
                return 0;
        }
        return 1;
    } else
        return 0;
}


int main()
{
    char *str1 = "121-168.1.1";
    char *str2 = "192.168.1.000001";
    char *str3 = NULL;
    char *timeout = " 1232323sdfasdf asdfasdf2 23r2";
    int t;

/*     printf("is ip address %s valid ? %d\n", str1, is_valid_ip(str1));
 *     printf("is ip address %s valid ? %d\n", str2, is_valid_ip(str2));
 */
    if ( 1 == sscanf(timeout, "%d", &t) ){
        printf("t:|||%d\n", t);
    }
    return 0;
}
