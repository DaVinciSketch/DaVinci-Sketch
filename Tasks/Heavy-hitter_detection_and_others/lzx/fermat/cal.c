#include <stdio.h>
#include <math.h>
int main()
{
    int result = (int)(pow(2, 9) * 10) % 11;
    printf("%d", result);
    return 0;
}