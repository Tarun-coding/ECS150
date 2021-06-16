#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct node {
	void *data;
	struct node *next;
};
typedef struct node node;

struct queue {
	unsigned int length;
	node *front, *rear;
};
typedef struct queue queue;

queue_t queue_create(void)
{
	queue_t newQueue = (queue_t)malloc(sizeof(queue));

	if (newQueue != NULL) {
		newQueue->length = 0;
		newQueue->front = newQueue->rear = NULL;
	}

	return newQueue;
}

int queue_destroy(queue_t queue)
{
	if (!queue || queue->length > 0) {
		return -1;
	}
	else {
		free(queue);
		return 0;
	}
}

int queue_enqueue(queue_t queue, void *data)
{
	node *newNode = (node*)malloc(sizeof(node));

	if (!queue || !data || !newNode) {
		return -1;
	}
	else {
		newNode->data = data;
		newNode->next = NULL;

		if (queue->length == 0) {
			queue->front = queue->rear = newNode;
		}
		else {
			queue->rear->next = newNode;
			queue->rear = newNode;
		}

		queue->length++;
		return 0;
	}
}

int queue_dequeue(queue_t queue, void **data)
{
	if (!queue || !data || queue->length == 0) {
		return -1;
	}
	else {
		node *removedNode;
		removedNode = queue->front;

		*data = removedNode->data;
		queue->front = queue->front->next;
		queue->length--;

		free(removedNode);
		removedNode = NULL;

		return 0;
	}
}

int queue_delete(queue_t queue, void *data)
{
	node *searchNode = NULL;
	node *prevNode = NULL;

	if (!queue || !data || queue->length == 0)
		return -1;

	searchNode = queue->front;

	while (data != searchNode->data && searchNode->next != NULL) {
		prevNode = searchNode;
		searchNode = searchNode->next;
	}

	if (searchNode->data == data) {
		if (queue->length == 1) {
			queue->front = NULL;
			queue->rear = NULL;
		} else if (searchNode == queue->front) {
			queue->front = searchNode->next;
		} else if ( searchNode == queue->rear) {
			prevNode->next = NULL;
		} else {
			prevNode->next = searchNode->next;
		}

		free(searchNode);
		searchNode = NULL;
		queue->length--;

		return 0;
	} else {
		return -1;
	}
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	node *searchNode = NULL;
	node *nextNode = NULL;

	if (!queue || !func)
		return -1;

	searchNode = queue->front;
	while (searchNode != NULL) {
		// Grab next node early in case func deletes the current node
		nextNode = searchNode->next;
		if (func(queue, searchNode->data, arg)) {
			*data = searchNode->data;
			return 1;
			break;
		}
		searchNode = nextNode;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (queue != NULL) {
		return queue->length;
	}
	else {
		return -1;
	}
}
