/*The first element in the queue is thread1. It creates thread 2 and yields to it 
 * so at the time of execution of thread 2, current thread=2 element in queue=[1]
 *next thread 2 creates thread 3, so current thread=2 elements in queue=[1,3]
  next thread 2 enters an infinite loop, yielding by preemption, current thread=1 elements in queue=[3,2]
  thread1 is printed and thread 1 is finished, current thread=3, elements in queue=[2,0]
  thread3 is printed and thread 3 is finished, current thread=2, elements in queue=[0]
  thread 2 yields to 0 because of preemption, current thread=0 elements in queue=[2]
  thread 0 prints we are in main
 * thread1
 * thread3
 * we are in main
 */

#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

int thread3(void)
{
//	uthread_yield();
	printf("thread%d\n", uthread_self());
	
	return 0;
}

int thread2(void)
{
uthread_create(thread3);
	
	//uthread_yield();	
	while(1);
	printf("thread%d\n", uthread_self());
	
	return 0;
}

int thread1(void)
{
	uthread_create(thread2);
	uthread_yield();
	
		
	printf("thread%d\n", uthread_self());
//	uthread_yield();
	return 1;
}

int main(void)
{
	int retval;
	uthread_start(1);
	uthread_join(uthread_create(thread1), &retval);
	
	printf("we are in main\n");
	
	uthread_stop();

	return 0;
}
