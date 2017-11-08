#include <stdio.h>
#include <stdlib.h>
int bss1;
int bss2;
int segment1 = 1;
int segment2 = 2;
int main()
{
    printf("bss 1 - %p\n", (void *) &bss1);
    printf("bss 2 - %p\n", (void *) &bss2);
    printf("data segment 1 - %p\n", (void *) &segment1);
    printf("data segment 2 - %p\n", (void *) &segment2);
    int stack1;
    int stack2;
    printf("stack 1 - %p\n", (void *) &stack1);
    printf("stack 2 - %p\n", (void *) &stack2);

    int* heap1 = malloc(sizeof(int));
    int* heap2 = malloc(sizeof(int));
    printf("heap 1 - %p\n", (void *) heap1);
    printf("heap 2 - %p\n", (void *) heap2);
    printf("heap 1 (pointer) - %p\n", (void *) &heap1);
    printf("heap 2 (pointer) - %p\n", (void *) &heap2);
    return 0;
}