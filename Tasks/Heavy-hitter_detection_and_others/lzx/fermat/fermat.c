#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE 3
#define PRIME_NUMBER 11

typedef struct HashBucket
{
    int count;
    int id;
} HashBucket;

typedef struct HashBucketArray
{
    HashBucket bucket1[ARRAY_SIZE]; // 哈希函数 1
    HashBucket bucket2[ARRAY_SIZE]; // 哈希函数 2
} HashBucketArray;

// 哈希函数 1
int hashFunction1(int key) 
{
    return key % ARRAY_SIZE;
}

// 哈希函数 2
int hashFunction2(int key) 
{
    return (key + 2) % ARRAY_SIZE;
}

void insert(int key, HashBucket bucket[], int (*hashFunction)(int)) 
{
    int index = hashFunction(key);
    bucket[index].id = (bucket[index].id + key) % PRIME_NUMBER;
    bucket[index].count += 1;
}

int diff(int id1, int id2) 
{
    int id_d = id1 - id2;
    if (id_d < 0)
    {
        id_d += PRIME_NUMBER;
    }
    return id_d;
}

typedef struct Node
{
    HashBucket data;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node* front;
    Node* rear;
} Queue;

void enqueue(Queue* queue, HashBucket bucket) 
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = bucket;
    newNode->next = NULL;
    if (queue->rear == NULL) 
    {
        queue->front = queue->rear = newNode;
        return;
    }
    queue->rear->next = newNode;
    queue->rear = newNode;
}

int isQueueEmpty(Queue* queue) 
{
    return (queue->front == NULL);
}

HashBucket dequeue(Queue* queue) 
{
    if (isQueueEmpty(queue)) 
    {
        // 队列为空，返回一个空的 HashBucket
        HashBucket emptyBucket = {0, 0}; // 假设空的 HashBucket 的 count 和 id 都为 0
        return emptyBucket;
    }

    Node* temp = queue->front;
    HashBucket data = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) 
    {
        queue->rear = NULL;
    }

    free(temp);
    return data;
}

int getQueueLength(Queue* queue) 
{
    int length = 0;
    Node* current = queue->front;
    while (current != NULL) 
    {
        length++;
        current = current->next;
    }
    return length;
}

int main() 
{
    int inputNumbers1[] = {5, 10, 9, 5, 5}; // 输入的数组 1
    int inputNumbers2[] = {5, 10, 9}; // 输入的数组 2

    HashBucketArray A = {0};
    HashBucketArray B = {0};
    HashBucketArray D = {0};
    //printf("%d", sizeof(A.bucket1) / sizeof(A.bucket1[0]));

    for (int i = 0; i < 5; i++) 
    {
        insert(inputNumbers1[i], A.bucket1, hashFunction1);
        insert(inputNumbers1[i], A.bucket2, hashFunction2);
    }
    for (int i = 0; i < 3; i++)
    {
        insert(inputNumbers2[i], B.bucket1, hashFunction1);
        insert(inputNumbers2[i], B.bucket2, hashFunction2);
    }

    for (int i = 0; i < 3; i++)
    {
        D.bucket1[i].count = A.bucket1[i].count - B.bucket1[i].count;
        D.bucket1[i].id = diff(A.bucket1[i].id, B.bucket1[i].id);

        D.bucket2[i].count = A.bucket2[i].count - B.bucket2[i].count;
        D.bucket2[i].id = diff(A.bucket2[i].id, B.bucket2[i].id);
    }

    Queue myQueue;
    myQueue.front = myQueue.rear = NULL;

    // 遍历 bucket1，将不为零的桶加入队列
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        if (D.bucket1[i].count != 0) 
        {
            enqueue(&myQueue, D.bucket1[i]);
        }
    }

    // 遍历 bucket2，将不为零的桶加入队列
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        if (D.bucket2[i].count != 0) 
        {
            enqueue(&myQueue,D.bucket2[i]);
        }
    }
    printf("length: %d\n", getQueueLength(&myQueue));

    while (!isQueueEmpty(&myQueue))
    {
        HashBucket popped = dequeue(&myQueue);
        if (popped.count != 0 || popped.id != 0) 
        {
            printf("Popped: Count = %d, ID = %d\n", popped.count, popped.id);
        } 
        else 
        {
            printf("Empty!\n");
        }
        /* 没有做纯净桶校验，这里差集没有做哈希映射，是一对一的 */
        // 计算差集 id
        int result = (int)(pow(popped.count, (PRIME_NUMBER - 2)) * popped.id) % PRIME_NUMBER;
        printf("%d\n", result);
    }

    // 打印结果 A
    printf("A\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, A.bucket1[i].count, A.bucket1[i].id);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, A.bucket2[i].count, A.bucket2[i].id);
    }

    // 打印结果 B
    printf("B\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, B.bucket1[i].count, B.bucket1[i].id);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, B.bucket2[i].count, B.bucket2[i].id);
    }

    // 打印差集
    printf("Differences\n");
    printf("Results for hash function 1:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, D.bucket1[i].count, D.bucket1[i].id);
    }

    printf("\nResults for hash function 2:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        printf("Index %d: Count = %d, ID = %d\n", i, D.bucket2[i].count, D.bucket2[i].id);
    }

    return 0;
}
