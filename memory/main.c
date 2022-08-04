#include <stdio.h>
#include <assert.h>
#include "memory.h"
#include <pthread.h>

struct test{
    int a;
    long int b;
    long long int c;
    float d;
    double e;
};

typedef struct test MyObject;

void *myThreadFun(void *vargp)
{

    // memory allocation : int

    printf("FROM THREAD:\n");
    printf("Allocating memory for an integer using new malloc function\n------------------------------\n");
    printf("\tRequesting memory of size : %zu bytes\n",sizeof(int));

    int *a = (int*)malloc_new(sizeof(int));
    *a = 10;

    printf("\tAllocated memory of size: %zu bytes at addr %p\n",sizeof(int),a);
    printf("\tThe value of the integer is: %d\n\n",*a);

    printf("\tFreeing the integer memory\n");
    free_new(a);
    printf("\tSuccessfully free'd %zu bytes from addr %p\n", sizeof(int),a);

    return NULL;
}
 
int main(void)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, myThreadFun, NULL);
    pthread_join(thread_id, NULL);

    // memory allocation : struct

    printf("\n\nFROM MAIN():\n");
    printf("Allocating memory for a struct using new malloc function\n------------------------------\n");
    printf("\tRequesting memory of size : %zu bytes\n",sizeof(MyObject));

    MyObject *my1 = malloc_new(sizeof(MyObject));
    my1->a = 10;
    my1->b = 10000;
    my1->c = 100000000;
    my1->d = 3.14;
    my1->e = 3.2322234234234234;

    printf("\tAllocated memory of size: %zu bytes at addr %p\n",sizeof(MyObject),my1);
    printf("\tThe contents of the struct are:\n\n");
    printf("\t {\n");
    printf("\t   a = %d\n",my1->a);
    printf("\t   b = %ld\n",my1->b);
    printf("\t   c = %lld\n",my1->c);
    printf("\t   d = %f\n",my1->d);
    printf("\t   e = %lf\n",my1->e);
    printf("\t }\n\n");
    
    printf("\tFreeing the struct memory\n");
    free_new(my1);
    printf("\tSuccessfully free'd %zu bytes from addr %p\n", sizeof(MyObject),my1);

    return 0;
}