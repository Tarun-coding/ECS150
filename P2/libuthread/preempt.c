#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<string.h>
#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

bool enable=false;
void alarm_handler(int signum){

preempt_enable();
	uthread_yield();
signum++;	

}
void preempt_start(void)
{
	
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler=alarm_handler;
	sigaction(SIGVTALRM,&sa,NULL);
	/* TODO */
}

void preempt_stop(void)
{
	/* TODO */
}

void preempt_enable(void)
{
	
 	if(enable){	
	struct itimerval timer;
	timer.it_value.tv_sec=0;
	timer.it_value.tv_usec=10000;

	setitimer(ITIMER_VIRTUAL,&timer,NULL);
	}
	/* TODO */
	
}

void preempt_disable(void)
{
	/* TODO */
}
void enableOrDisable(void){
	enable=true;
}
