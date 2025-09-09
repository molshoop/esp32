#include <stdio.h>

void app_main(void)
{
    printf("Hallo wereld\n");
    volatile int a = 0;
    a = a + 1;
    printf("%d\n", a);
}