#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#define MAX_SIZE 10000

typedef struct {
    int items[MAX_SIZE];
    int front;
    int numItems;
} Queue;

void initializeQueue(Queue* q);
bool isEmpty(Queue* q);
bool isFull(Queue* q);
void enqueue(Queue* q, int value);
int poll(Queue* q);
int peek(Queue* q);

#endif // QUEUE_H