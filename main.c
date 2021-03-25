// Based on https://c9x.me/articles/gthreads/code0.html
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

struct timespec start, end;

// Dummy function to simulate some thread work
void f(void) {
  static int x;
  int i, id;

  id = ++x;
  while (true) {
    printf("F Thread id = %d, val = %d BEGINNING\n", id, ++i);
    uninterruptibleNanoSleep(0, 50000000);
    printf("F Thread id = %d, val = %d END\n", id, ++i);
    uninterruptibleNanoSleep(0, 50000000);
  }
}

// Dummy function to simulate some thread work
void g(void) {
  static int x;
  int i, id;

  id = ++x;
  while (true) {
    printf("G Thread id = %d, val = %d BEGINNING\n", id, ++i);
    uninterruptibleNanoSleep(0, 50000000);
    printf("G Thread id = %d, val = %d END\n", id, ++i);
    uninterruptibleNanoSleep(0, 50000000);
  }
}

void sigint(int sig) {
    printf("\n\n                      Statistics\n");
    
    // for 100% work
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t time = (double)(end.tv_sec - start.tv_sec) * 1000000 + (double)(end.tv_nsec - start.tv_nsec) / 1000;
    printf("                  Time: %f sec.\n\n", (double)time/1000000);
    
    //double timeArr[10];
    //double sum = 0.0;
    for (int i = 0; i < MaxGThreads; i++) {
			//timeArr[i] = gttbl[i].priority;
			printf("                     Thread id: %d\n", i);
			printf("                  Thread priority: %d\n", gttbl[i].priority);
			printf("      Run                 |       Wait\n");
			printf("Total run time:  %f | Total wait time: %f\n", gttbl[i].totalRunTime, gttbl[i].totalWaitTime);
			printf(" Min. run time:  %f |  Min. wait time: %f\n", gttbl[i].minRunTime, gttbl[i].minWaitTime);
			printf(" Max. run time:  %f |  Max. wait time: %f\n", gttbl[i].maxRunTime, gttbl[i].maxWaitTime);
			printf(" Avg. run time:  %f |  Avg. wait time: %f\n", gttbl[i].totalRunTime / gttbl[i].countRunAvg, gttbl[i].totalWaitTime / gttbl[i].countWaitAvg);
			printf("\n");
			//sum += gttbl[i].totalRunTime;
    }
    //printf("Sum run time: %f\n", sum); 
    
    /*printf("                     Priority\n");
    printf("                  Time: %f sec.\n\n", (double)time/1000000);
    for (int i = 0; i < MaxGThreads; i++) {
			printf("Thread id: %d, Priority: %d, run time: %f\n", i, gttbl[i].priority, gttbl[i].totalRunTime);
    }
    printf("\n");*/

    exit(0);
}


int main(int argc, char* argv[]) {

	if (atoi(argv[1]) == 1) { //rr
		printf("rr\n");
		
	  gtinit();
		gtgo(f);
		gtgo(f);
		gtgo(g);
		gtgo(g);
		gtret(1);
	}
	else if (atoi(argv[1]) == 2) { //pri
		printf("pri\n");
	}
	else if (atoi(argv[1]) == 3) { //ls
		printf("ls\n");
	}
	return 0;
  
  signal(SIGINT, sigint);

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
  gtinit(10);		// initialize threads, see gthr.c
  gtgo(f, 9);		// set f() as first thread
  gtgo(f, 8);		// set f() as second thread
  gtgo(g, 7);		// set g() as third thread
  gtgo(g, 6);		// set g() as fourth thread
  
/*  gtgo(f, 5);
  gtgo(f, 4);
  gtgo(f, 3);
  gtgo(g, 2);
  gtgo(g, 1);
  gtgo(g, 0);*/
  
  gtret(1);		// wait until all threads terminate
}
