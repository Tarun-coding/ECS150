#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

static int inc_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int inc = (int)(long)arg;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += inc;

	return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int match = (int)(long)arg;
	(void)q; //unused

	if (*a == match)
		return 1;

	return 0;
}

/* Test delete */
void test_delete(void)
{
	queue_t q;
	int *ptr;
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

	fprintf(stderr, "*** TEST delete ***\n");

	ptr = (int*)malloc(sizeof(int));
	*ptr = 0;

	q = queue_create();
	for (unsigned int i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	// Test delete from middle
	queue_delete(q, (void*)(long)&data[3]);
	queue_iterate(q, find_item, (void*)4, (void**)&ptr);
	TEST_ASSERT(*ptr != data[3]);

	// Test delete from front
	queue_delete(q, (void*)(long)&data[0]);
	queue_iterate(q, find_item, (void*)1, (void**)&ptr);
	TEST_ASSERT(*ptr != data[0]);

	// Test delete from rear
	queue_delete(q, (void*)(long)&data[9]);
	queue_iterate(q, find_item, (void*)10, (void**)&ptr);
	TEST_ASSERT(*ptr != data[9]);

	// Remove all but one node
	queue_delete(q, (void*)(long)&data[1]);
	queue_delete(q, (void*)(long)&data[2]);
	queue_delete(q, (void*)(long)&data[4]);
	queue_delete(q, (void*)(long)&data[5]);
	queue_delete(q, (void*)(long)&data[6]);
	queue_delete(q, (void*)(long)&data[7]);
	TEST_ASSERT(queue_length(q) == 1);

	// Test delete last node
	queue_delete(q, (void*)(long)&data[8]);
	TEST_ASSERT(queue_length(q) == 0);

}

void test_iterator(void)
{
	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	size_t i;
	int *ptr;

	fprintf(stderr, "*** TEST iterator ***\n");

	/* Initialize the queue and items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Add value '1' to every item of the queue, delete item '42' */
	queue_iterate(q, inc_item, (void*)1, NULL);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(queue_length(q) == 9);

	/* Find and get the item which is equal to the value '5' */
	ptr = NULL;		// result pointer *must* be reset first
	queue_iterate(q, find_item, (void*)5, (void**)&ptr);
	TEST_ASSERT(ptr != NULL);
	TEST_ASSERT(*ptr == 5);
	TEST_ASSERT(ptr == &data[3]);
}

/* Create queue*/
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_iterator();
	test_delete();

	return 0;
}
