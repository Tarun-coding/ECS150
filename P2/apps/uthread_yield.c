/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread1
 * thread2
 * thread3
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void)
{
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 1;
}

int thread2(void)
{
	int *retVal = (int*)malloc(sizeof(int));
	int child = uthread_create(thread3);
	*retVal = 0;

	uthread_yield();
	printf("thread%d\n", uthread_self());

	uthread_join(child, retVal);

	return 1 + *retVal;
}

int thread1(void)
{
	int *retVal = (int*)malloc(sizeof(int));
	int child = uthread_create(thread2);
	*retVal = 0;

	uthread_yield();
	printf("thread%d\n", uthread_self());
	uthread_yield();

	uthread_join(child, retVal);

	return 1 + *retVal;
}

/* Tests yield as well as join. Each successful thread will return
 * and add 1 more to the retVal count. */
int main(void)
{
	int *retVal = (int*)malloc(sizeof(int));
	int child;
	*retVal = 0;

	uthread_start(0);
	child = uthread_create(thread1);
	uthread_join(child, retVal);
	uthread_stop();

	printf("Successful threads: %d\n", *retVal);

	return 0;
}
