# User-level thread library

## Summary

This is a library for the creation and management of user-level threads.

## Implementation

Our implementation was developed in 4 phases:

1. Queue data structure
2. Thread management and Queue utilizations
3. Thread preemption

### Phase 1: Queue data structure

To begin we implemented a FIFO queue. The standard functions were implemented
such as 'queue_enqueue()', 'queue_dequeue()', etc. Also included is the function
'queue_iterate()'. This function takes in a function as a parameter to act as a
callback. This allows the creation of functions outside of the API to act upon
the queue utilizing this iterator.

A singly-linked list was chosen as the data structure on which the queue would
be built. All functions, aside from delete and iterate, could be made with O(1)
complexity. Additionally, a linked list allowed for more flexibility in terms
of size.


### Preliminary information for uthread.c:
I used a struct TCB which contains the thread's TID, context, stack pointer, joining thread's tid, and retval. 
Global variables: 1) TCB called currentThread to keep track of the thread we are in at any moment.  
                  2)An array called TCBs which keeps track of all the threads that have been made,
                  providing us easy acess to the member variables to all the threads.  
                  3)An array ctx keeping track of all of the contexts of the threads.
                  4)Queues: q, zombieQ, blockedQ. q contains all the threads ready for execution, 
                  blockedQ contains all the TCBs in the blocked state and the zombieQ contains the TCBs ready to be joined.
                  5)variable i: this carries the TID for the very next TCB needed to be created.

created functions:1) find_item(queue_t q, void* data, void* arg). 
                 This function is used to find a TCB in the queue which has the same joiningTID as specified by arg.
                  2) find_itemTID(queue_t q, void* data, void* arg). 
                  This function is used to find a TCB in the queue which has the same TID as specified by arg.
                  
                  
### Phase 2: Thread and queue management
    ## thread creation
    
   The first thread that we initialize is the main thread in the start function. It's initialized as TCBs[0].
   For every new thread that is being created we create a new element in TCBs as TCBs[i]. 
   For instance, since the main thread is already initialized as TCBs[0], the very next thread created would be 
   initialized in TCBs[1]. We intialize this TCB by generating a fake context.
   
    ## thread yielding
   Firstly we call uthread_self() to get the tid of the thread being yielded, we call this previousTID . 
   This tid could be used to acess all of the member variables of the current thread. We first enque the 
   currrent thread back into the ready queue, making it go to the back of the order. Then we dequeue the 
   queue for the next TCB and call this currentThread. Now we use the previousTID in order to save the 
   context of the current thread by first acessing it from the array and then accessing its context member: 
   TCBs[previousTID].context. The other thread that we will switch contexts to will simply be the currentThread.
   
    ## join -> exit functions
   For the first part of the join function we use similar mechanisms as thread yeilding, except this time 
   the thread in enqued into the blockedQ. This way it stays away from being scheduled when yield is called.
   When the thread is done executing it reaches the exit function. Here our first objective is to see if 
   there's a thread in the blockedQ which is waiting for the current thread to be finished. To do this, we
   use the function queue iterate, and within which we're going to use find_item. Now we will iterate 
   through every TCB in the queue until we find a TCB which has the same joining TID as the TID of the 
   exiting thread. Once it has been found, we will simply enqueue it back into the ready queue and delete
   it from the blockedQ. Finally, the current thread will be enqueued into the zombieQ which will wait 
   for the joining thread to collect it.
   
    ##  exit->join functions
   Now for the second part, the join function will check for the Tid of every element in the zombieQ until
   it finds its joining thread. It does this by quing queue_iterate and find_itemTID function within it.
   Once it has found the TCB, it will make  retval equal to the TCB's memeber variable retval. Next, it
   will delete the element from the zombieQ and finally destroy the TCB's stack. 


### Phase 3: Preempting threads

After initializing the sig handler in preempt_start, the preempt_enable() function, sets the timer on call. 
It runs from 0 to 10 milli seconds. Thus, it will stop 100 times every second. preempt_enable is called
right before the execution of the thread in uthread_ctx_bootstrap in context.c. So a timer is started
before the execution of the thread. Once the timer runs out, the alarm_handler() function will take
action. First, alarm_handler will start a new timer by calling preempt_enable(), and then it will 
yeild to the next thread in the queue. The reason for calling preempt enable before yielding is 
because when the thread pops back up from the queue on the next iteration, it would have already 
executed the preempt_enable() in the uthread_ctx_bootstrap function, so it would run without a timer. 
Thus, by execution preempt_enable right before yielding we manage to start a new timer that will
be used when executing on the next iteration.

### Proof for Preempt tester file
  
During the execution of the main function: the current thread = main, ready queue=[thread1], blocked queue=[]
After the execution of the main function: the current thread = thread1, ready queue=[],blocked queue[main]
thread1 creates thread2 and adds it to the ready queue: current thread= thread1, ready queue=[thread2], blocked queue=[main]
thread1 yields to thread2: current thread=thread2, ready queue = [thread1], blocked queue=[main]
thread2 creates thread3: current thread = thread2, ready queue= [thread1,thread3], blocked queue=[main]

After encountering an infinite loop, thread2 yields control: 
current thread = thread1, ready queue=[thread3,thread2], blocked queue=[main]
thread1 prints "thread1" and exits causing main to come back to the ready queue:
current thread = thread3, ready queue=[thread2,main], blocked queue=[]

thread3 prints "thread3" and let's the next thread take control:
current thread=thread2, ready queue=[main], blocked queue=[]

Since thread2 is still in the infinite loop, it yields control again: 
current thread = main, ready queue=[thread2],blocked queue=[]
main prints "we are in main"

Thus, output should be:
thread1
thread3
we are in main

