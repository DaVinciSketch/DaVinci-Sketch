#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BIG_ARRAY_SIZE 3
#define PRIME_NUMBER 11

// 哈希函数1
int hashFunction1(int key) 
{
    return key % BIG_ARRAY_SIZE;
}

// 哈希函数2
int hashFunction2(int key) 
{
    return (key + 2) % BIG_ARRAY_SIZE;
}

void insert(int key, int countArray[], int idArray[], int (*hashFunction)(int)) 
{
    int index = hashFunction(key);
    idArray[index] = (idArray[index] + key) % PRIME_NUMBER;
    countArray[index] += 1;
}

int diff(int idArray1, int idArray2) 
{
    int idArray_d = idArray1 - idArray2;
    if (idArray_d < 0)
    {
        idArray_d += PRIME_NUMBER;
    }
    return idArray_d;
}

int main() 
{
    int inputNumbers1[] = {5, 10, 9, 5, 5}; // 输入的数列1
    int inputNumbers2[] = {5, 10, 9}; // 输入的数列2

    int countArray11[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的count数组
    int idArray11[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的id数组

    int countArray12[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的count数组
    int idArray12[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的id数组

    int countArray21[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的count数组
    int idArray21[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的id数组

    int countArray22[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的count数组
    int idArray22[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的id数组

    // 差
    int countArray_d1[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的count数组
    int idArray_d1[BIG_ARRAY_SIZE] = {0}; // 第一个哈希函数的id数组

    int countArray_d2[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的count数组
    int idArray_d2[BIG_ARRAY_SIZE] = {0}; // 第二个哈希函数的id数组


    for (int i = 0; i < 5; i++) 
    {
        insert(inputNumbers1[i], countArray11, idArray11, hashFunction1);
        insert(inputNumbers1[i], countArray12, idArray12, hashFunction2);
    }
    for (int i = 0; i < 3; i++)
    {
        insert(inputNumbers2[i], countArray21, idArray21, hashFunction1);
        insert(inputNumbers2[i], countArray22, idArray22, hashFunction2);
    }

    for (int i = 0; i < 3; i++)
    {
        countArray_d1[i] = countArray11[i] - countArray21[i];
        idArray_d1[i] = diff(idArray11[i], idArray21[i]);

        countArray_d2[i] = countArray12[i] - countArray22[i];
        idArray_d2[i] = diff(idArray12[i], idArray22[i]);
    }

    // 打印结果 A
    printf("A\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray11[i], idArray11[i]);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray12[i], idArray12[i]);
    }

    // 打印结果 B
    printf("B\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray21[i], idArray21[i]);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray22[i], idArray22[i]);
    }

    // 打印差集
    printf("Differences\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray_d1[i], idArray_d1[i]);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < BIG_ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, countArray_d2[i], idArray_d2[i]);
    }

    // 计算差集 id
    int result = (int)(pow(2, (PRIME_NUMBER-2)) * 10) % PRIME_NUMBER;
    printf("%d", result);

    return 0;
}
