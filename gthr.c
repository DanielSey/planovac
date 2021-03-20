#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "gthr.h"
#include "gthr_struct.h"

// function triggered periodically by timer (SIGALRM)
void gthandle(int sig) {
  gtyield();
}

// initialize first thread as current context
void gtinit(int priority) {
  gtcur = & gttbl[0];			// initialize current thread with thread #0
  gtcur -> st = Running;		// set current to running
  
  // thread is running, start run clock
  clock_gettime(CLOCK_MONOTONIC_RAW, &gtcur->startRunClock);
  
  // set up default values
  gtcur -> totalRunTime = 0.0;
  gtcur -> minRunTime = 99999.0;
  gtcur -> maxRunTime = 0.0;
  gtcur -> countRunAvg = 0;
  
  gtcur -> totalWaitTime = 0.0;
  gtcur -> minWaitTime = 99999.0;
  gtcur -> maxWaitTime = 0.0;
  gtcur -> countWaitAvg = 0;
  
  gtcur -> priority = priority;
  gtcur -> actualPriority = priority;
  
  signal(SIGALRM, gthandle);	// register SIGALRM, signal from timer generated by alarm
}

// exit thread
void __attribute__((noreturn)) gtret(int ret) {
  if (gtcur != & gttbl[0]) {	// if not an initial thread,
    gtcur -> st = Unused;		// set current thread as unused
    gtyield();					// yield and make possible to switch to another thread
    assert(!"reachable");		// this code should never be reachable ... (if yes, returning function on stack was corrupted)
  }
  while (gtyield());			// if initial thread, wait for other to terminate
  exit(ret);
}

// switch from one thread to other
bool gtyield(void) {
  struct gt * p;
  struct gtctx * old, * new;

  resetsig(SIGALRM);			// reset signal

  p = gtcur;
  
  while (true) {
  	if (p->st == Ready && p->actualPriority <= 0) {
    	break;
  	}
  	else {
			p->actualPriority--;
		}
    if (++p == & gttbl[MaxGThreads])
      p = & gttbl[0];
  }
 	p->actualPriority = p->priority;

  if (gtcur -> st != Unused) {			// switch current to Ready and new thread found in previous loop to Running
    gtcur -> st = Ready;
  }
  
  // stop run time of previous thread
  clock_gettime(CLOCK_MONOTONIC_RAW, &gtcur->endRunClock);
  uint64_t time = (double)(gtcur->endRunClock.tv_sec - gtcur->startRunClock.tv_sec) * 1000000 + (double)(gtcur->endRunClock.tv_nsec - gtcur->startRunClock.tv_nsec) / 1000;
  double timeSec = (double)time / 1000000;
  gtcur->totalRunTime += timeSec;
  gtcur->countRunAvg++;
  
  if (timeSec < gtcur->minRunTime) {
     gtcur->minRunTime = timeSec;
  }
  if (timeSec > gtcur->maxRunTime) {
     gtcur->maxRunTime = timeSec;
  }
  
  // start wait time of previous thread
  clock_gettime(CLOCK_MONOTONIC_RAW, &gtcur->startWaitClock);
  
  p -> st = Running;
  
  // stop wait time of new thread
  clock_gettime(CLOCK_MONOTONIC_RAW, &p->endWaitClock);
  uint64_t time2 = (double)(p->endWaitClock.tv_sec - p->startWaitClock.tv_sec) * 1000000 + (double)(p->endWaitClock.tv_nsec - p->startWaitClock.tv_nsec) / 1000;
  double timeSec2 = (double)time2 / 1000000;
  p->totalWaitTime += timeSec2;
  p->countWaitAvg++;
  
  if (timeSec2 < p->minWaitTime) {
     p->minWaitTime = timeSec2;
  }
  if (timeSec2 > p->maxWaitTime) {
     p->maxWaitTime = timeSec2;
  }
  
  // start run time of new thread
  clock_gettime(CLOCK_MONOTONIC_RAW, &p->startRunClock);
  
  old = & gtcur -> ctx;					// prepare pointers to context of current (will become old) 
  new = & p -> ctx;						// and new to new thread found in previous loop
  gtcur = p;							// switch current indicator to new thread
  gtswtch(old, new);					// perform context switch (assembly in gtswtch.S)
  
  return true;
}

// return function for terminating thread
void gtstop(void) {
  gtret(0);
}

// create new thread by providing pointer to function that will act like "run" method
int gtgo(void( * f)(void), int priority) {
  char * stack;
  struct gt * p;

  for (p = & gttbl[0];; p++)			// find an empty slot
    if (p == & gttbl[MaxGThreads])		// if we have reached the end, gttbl is full and we cannot create a new thread
      return -1;
    else if (p -> st == Unused)
      break;								// new slot was found

  stack = malloc(StackSize);			// allocate memory for stack of newly created thread
  if (!stack)
    return -1;

  *(uint64_t * ) & stack[StackSize - 8] = (uint64_t) gtstop;	//	put into the stack returning function gtstop in case function calls return
  *(uint64_t * ) & stack[StackSize - 16] = (uint64_t) f;		//  put provided function as a main "run" function
  p -> ctx.rsp = (uint64_t) & stack[StackSize - 16];			//  set stack pointer
  p -> st = Ready;												//  set state

  // thread is ready, start waiting clock
  clock_gettime(CLOCK_MONOTONIC_RAW, &p->startWaitClock);
  
  // set up default values
  p -> totalRunTime = 0.0;
  p -> minRunTime = 99999.0;
  p -> maxRunTime = 0.0;
  p -> countRunAvg = 0;
  
  p -> totalWaitTime = 0.0;
  p -> minWaitTime = 99999.0;
  p -> maxWaitTime = 0.0;
  p -> countWaitAvg = 0;
  
  p -> priority = priority;
  p -> actualPriority = priority;
  
  return 0;
}

void resetsig(int sig) {
  if (sig == SIGALRM) {
    alarm(0);			// Clear pending alarms if any
  }

  sigset_t set;				// Create signal set
  sigemptyset( & set);		// Clear it
  sigaddset( & set, sig);	// Set signal (we use SIGALRM)

  sigprocmask(SIG_UNBLOCK, & set, NULL);	// Fetch and change the signal mask

  if (sig == SIGALRM) {
    // Generate alarms
    ualarm(500, 500);		// Schedule signal after given number of microseconds
  }
}

int uninterruptibleNanoSleep(time_t sec, long nanosec) {
  struct timespec req;
  req.tv_sec = sec;
  req.tv_nsec = nanosec;

  do {
    if (0 != nanosleep( & req, & req)) {
      if (errno != EINTR)
        return -1;
    } else {
      break;
    }
  } while (req.tv_sec > 0 || req.tv_nsec > 0);
  return 0; /* Return success */
}
