#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"
/* TODO */

static uthread_ctx_t ctx[USHRT_MAX];
int i=1;
struct TCB{
	int TID;
	uthread_ctx_t context;
	void* stackPointer;
	int state;//for now 0:running, 1:ready, 2:exited
	int joiningTID;
	int retval;

};
struct TCB TCBs[USHRT_MAX];
struct TCB currentThread;
queue_t q;
queue_t blockedQ;
queue_t zombieQ;
//q = queue_create();

/*
static int printFunction(queue_t q, void *data,void *arg){
	struct TCB *a=(struct TCB*)data;
	printf("%d\n",(*a).TID);
}
*/

static int find_item(queue_t q,void * data,void *arg)
{
	struct TCB *a=(struct TCB*)data;
	int match=(int)(long)arg;
//	int match=(intptr_t)arg;
	(void)q;
	if((*a).joiningTID==match){
		return 1;
	}
	return 0;
}

static int find_itemTID(queue_t q,void * data,void *arg)
{
	struct TCB *a=(struct TCB*)data;
	int match=(int)(long)arg;
//	int match=(intptr_t)arg;
	(void)q;
	if((*a).TID==match){
		return 1;
	}
	return 0;
}
//static int
int uthread_start(int preempt)
{
	if(preempt==1){
	enableOrDisable();
	preempt_start();
	}	
	/* TODO */
	struct TCB mainTCB;
	mainTCB.TID=0;
	mainTCB.state=0;//it is running
	TCBs[0]=mainTCB;
	currentThread=mainTCB;
	q=queue_create();
	blockedQ=queue_create();
	zombieQ=queue_create();
//code to delete
	
	
		return 0;
}

int uthread_stop(void)
{

	if(queue_length(q)>0){
		return -1;
	}
	/* TODO */
	return 0;
}

int uthread_create(uthread_func_t func)
{
		
	//struct TCB *ptrTwo; 
	//queue_dequeue(q, (void**)&ptrTwo);
	
	//printf("%d is the tid for the main function\n",(*ptrTwo).TID);
	void* stackPointer=uthread_ctx_alloc_stack();	
	uthread_ctx_init(&ctx[i],stackPointer,func);
	TCBs[i].TID=i;
	TCBs[i].context=ctx[i];
	TCBs[i].state=i;
	TCBs[i].stackPointer=stackPointer;
	queue_enqueue(q,&TCBs[i]);
	i++;
	return i-1;	
	
//	uthread_ctx_switch(&ctx[0],&TCBs[1].context);
	
//	printf("still in create\n");

	
	/* TODO */
	return -1;
}

void uthread_yield(void)
{
		
 		
	struct TCB* ptrTwo;
	int previousTID=uthread_self();
	queue_enqueue(q,&TCBs[previousTID]);
	queue_dequeue(q,(void**)&ptrTwo);
	currentThread=(*ptrTwo);
	
	uthread_ctx_switch(&TCBs[previousTID].context,&currentThread.context);
	
		
		
	/* TODO */
}

uthread_t uthread_self(void)
{
	return currentThread.TID;
	/* TODO */
	return -1;
}

void uthread_exit(int retval)

{
	/* TODO */
	

	
	//printf("in exit: %d\n",retval);
	struct TCB* ptrThree;
	struct TCB* ptr;
	int previousTID=uthread_self();
	//queue_enqueue(q,&TCBs[previousTID]);
	TCBs[previousTID].retval=retval;
	if(queue_iterate(blockedQ,find_item,(void*)(intptr_t)previousTID,(void**)&ptr)==1){
		//enque TCB onto q
		//printf("we are in iterate\n");
		int tid=(*ptr).TID;
		queue_delete(blockedQ,ptr);
		queue_enqueue(q,&TCBs[tid]);

		
	}
		
	queue_dequeue(q,(void**)&ptrThree);
	currentThread=(*ptrThree);
	
	
	queue_enqueue(zombieQ,&TCBs[previousTID]);
//	printf("%d\n",previousTID);
	uthread_ctx_switch(&TCBs[previousTID].context,&currentThread.context);
//code to delete
	//int x=retval;
	//x++;

}

int uthread_join(uthread_t tid, int *retval)
{

	if(tid==0){
		return -1;
	}
	if(currentThread.TID==tid){
		return -1;
	}
	struct TCB* ptrTwo;
	struct TCB* ptrFive;
	int previousTID=uthread_self();
	TCBs[previousTID].joiningTID=tid;
	queue_enqueue(blockedQ,&TCBs[previousTID]);
		
	queue_dequeue(q,(void**)&ptrTwo);
	currentThread=(*ptrTwo);
	
	uthread_ctx_switch(&TCBs[previousTID].context,&currentThread.context);
	//printf("this happens after everything else\n");
 //code to delete

	
	if(queue_iterate(zombieQ,find_itemTID,(void*)(intptr_t)tid,(void**)&ptrFive)==1){
		//printf("we are in iterate\n");
		//enque TCB onto q
		int tidTwo=(*ptrFive).TID;
		if(retval!=NULL){
			*retval=TCBs[tidTwo].retval;
		}
		queue_delete(zombieQ,ptrFive);
		
		 uthread_ctx_destroy_stack(TCBs[tidTwo].stackPointer);
		//queue_enqueue(q,&TCBs[tid]);

		
	}else{
		return -1;
	}
	
	//ptrFive=&TCBs[tid];
	//queue_delete(zombieQ,&ptrFive);

//queue_iterate(zombieQ,printFunction,NULL,NULL);

	//printf("%d\n",queue_length(zombieQ));
      	
  
	return 0;
}

