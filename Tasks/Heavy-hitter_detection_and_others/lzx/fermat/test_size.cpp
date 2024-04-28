#include <iostream>
using namespace std;

struct HashBucket
{
    int count;
    char id[13];
};

int main()
{
     // 生成1个HashBucket实例
    const int num_buckets = 1;
    HashBucket buckets[num_buckets];
    for (int i = 0; i < num_buckets; i++) 
    {
        buckets[i].count = 1; // 生成随机的count值
        for (int j = 0; j < 13; j++) 
        {
            buckets[i].id[j] = rand() % 256; // 生成随机的8比特整数作为id的每个字节
        }
    }
    printf("%d", sizeof(buckets[0].id));
}