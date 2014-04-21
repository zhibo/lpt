#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void process(char *p, char **q)
{
    printf("addr of p: %p\n", p);
    *q = p;
    (*q)[1] = '\0';
    printf("*q is: %s\n", *q);
    printf("addr of q: %p\n", *q);
    *q = p + 1;
    printf("addr of q: %p\n", *q);
    *q = p + 2;
    printf("addr of q: %p\n", *q);

    printf("p is: %s\n", p);
/*     if ( !*q ){
 *         printf("p is null\n");
 *     }
 *     if ( 0 == strlen(*q)){
 *         printf("p size is 0\n");
 *     }
 */

}


void process2(char *p, char *q)
{
    printf("addr of p: %p\n", p);
    q = p;
    q[1] = '\0';
    printf("*q is: %s\n", q);
    printf("addr of q: %p\n", q);
    q = p + 1;
    printf("addr of q: %p\n", q);
    q = p + 2;
    printf("addr of q: %p\n", q);

    printf("p is: %s\n", p);
/*     if ( !*q ){
 *         printf("p is null\n");
 *     }
 *     if ( 0 == strlen(*q)){
 *         printf("p size is 0\n");
 *     }
 */

}

void reverse(char *p, char **q)
{
    int i = 0, len = strlen(p);
    int j = len - 1;
    printf("(in)addr of q: %p\n", *q);
    *q = (char*)malloc(sizeof(char) * 5); 
    printf("(in)addr of q: %p\n", *q);
    for (i = 0; i < len; i++){
        printf("i: %d, j: %d\n", i, j);
        (*q)[i] = p[j--];
    }
}

void reverse2(char *p, char *q)
{
    int i = 0, len = strlen(p);
    int j = len - 1;
    printf("(in)addr of q: %p\n", q);
    for (i = 0; i < len; i++){
        //printf("i: %d, j: %d\n", i, j);
        q[i] = p[j--];
    }
}

int main()
{
    char *p = (char*)malloc(sizeof(char) * 5); 
    strcpy(p, "abc");
    //char *q = (char*)malloc(sizeof(char) * 5); 
    char *q = NULL;

    printf("(pre)addr of q: %p\n", q);
    //process(p, &q);
    //process2(p, q);
    reverse(p, &q);
    //reverse2(p, q);
    printf("(post)addr of q: %p\n", q);

    printf("p is: %s\n", p);
    printf("q is: %s\n", q);

    return 0;
}
