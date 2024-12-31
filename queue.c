// Jan C. Bierowiec
// Thomas Noone
// Operating Systems Final Project
// Round Robin Scheduling Algorithm

#include <stdbool.h>
#include <stdio.h>
#include "queue.h"

void initializeQueue(Queue* q)
{
    q->front = 0;
    q->numItems = 0;
}

bool isEmpty(Queue* q)
{
    return q->numItems == 0;
}

bool isFull(Queue* q)
{
    return q->numItems == MAX_SIZE;
}

void enqueue(Queue* q, int value)
{
    if (isFull(q))
    {
        // adjusted to output the value
        printf("Queue is full, cannot enqueue %d\n", value);
        return;
    }
    // q should also point to numItems (that was 1 error)
    q->items[(q->front + q->numItems) % MAX_SIZE] = value;
    q->numItems++;
}

int poll(Queue* q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty, cannot poll.\n");
        return -1;
    }
    int value = q->items[q->front];
    q->front = (q->front + 1) % MAX_SIZE;

    //front = n;
    q->numItems--;
    return value;
}

int peek(Queue* q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty, cannot peek.\n");
        return -1;
    }
    return q->items[q->front];
}
