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
#include <string.h>

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
    printf("\n\n                     Statistics\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t time = (double)(end.tv_sec - start.tv_sec) * 1000000 + (double)(end.tv_nsec - start.tv_nsec) / 1000;
    printf("                 Time: %f sec.\n", (double)time/1000000);
		switch(stype) {
			case 1:
  			printf("              Planovac: Round-Robin (RR)\n");
				break;
			case 2:
				printf("       Planovac: Round-Robin with priorities (PRI)\n");
				break;
			case 3:
				printf("          Planovac: Lottery Scheduling (LS)\n");
				break;
		}
    printf("\n");
    
    for (int i = 0; i < MaxGThreads; i++) {
			printf("                     Thread id: %d\n", i);
			switch(stype) {
				case 2:
					printf("                  Thread priority: %d\n", gttbl[i].priority);
					break;
				case 3:
					printf("               Thread tickets: %d - %d (%d)\n", gttbl[i].tickets[0], gttbl[i].tickets[1], (gttbl[i].tickets[1] - gttbl[i].tickets[0]) + 1);
					break;
			}
			printf("      Run                 |       Wait\n");
			printf("Total run time:  %f | Total wait time: %f\n", gttbl[i].totalRunTime, gttbl[i].totalWaitTime);
			printf(" Min. run time:  %f |  Min. wait time: %f\n", gttbl[i].minRunTime, gttbl[i].minWaitTime);
			printf(" Max. run time:  %f |  Max. wait time: %f\n", gttbl[i].maxRunTime, gttbl[i].maxWaitTime);
			printf(" Avg. run time:  %f |  Avg. wait time: %f\n", gttbl[i].totalRunTime / gttbl[i].countRunAvg, gttbl[i].totalWaitTime / gttbl[i].countWaitAvg);
			printf("\n");
    }
    
    exit(0);
}


int main(int argc, char* argv[]) {
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
 	signal(SIGINT, sigint);
 	int type = 0;
 	
 	if (argc != 2) {
 		printf("you must choose one param: RR / PRI / LS\n");
		return 0;
 	}
 	
	if (!strcmp(argv[1], "RR")) 
		type = 1;
	else if (!strcmp(argv[1], "PRI"))
		type = 2;
	else if (!strcmp(argv[1], "LS")) 
		type = 3;
	else {
		printf("wrong params!\n");
		printf("you must choose one param: RR / PRI / LS\n");
		return 0;
	}
  
  gtinit(type, 8, 0, 1);		// initialize threads, see gthr.c
  gtgo(f, 6, 2, 20);		// set f() as first thread
  gtgo(f, 4, 21, 80);		// set f() as second thread
  gtgo(g, 2, 81, 85);		// set g() as third thread
  gtgo(g, 0, 86, 100);		// set g() as fourth thread
  
  /*gtgo(f, 5);
  gtgo(f, 4);
  gtgo(f, 3);
  gtgo(g, 2);
  gtgo(g, 1);
  gtgo(g, 0);*/
  
  gtret(1);		// wait until all threads terminate
}
