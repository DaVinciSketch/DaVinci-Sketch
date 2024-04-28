#include <stdio.h>

typedef struct HashBucket
{
    int count;
    int id;
} HashBucket;

int main()
{
    printf("%d", sizeof(HashBucket));
    return 0;
}