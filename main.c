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

struct timespec start, end; // for 100% work

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
    printf("\n\nStats\n\n");
    double sum = 0.0;
    for (int i = 0; i < MaxGThreads; i++) {
	printf("Thread id:%d\n", i);
	printf("Total runtime: %f\n", gttbl[i].totalTime);
	printf("Min. time: %f\n", gttbl[i].minTime);
	printf("Max. time: %f\n", gttbl[i].maxTime);
	printf("Avg. time: %f\n\n", gttbl[i].totalTime / gttbl[i].countAvg);
	sum += gttbl[i].totalTime;
    }
    printf("Sum: %f\n", sum); 

    // for 100% work
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t time = (double)(end.tv_sec - start.tv_sec) * 1000000 + (double)(end.tv_nsec - start.tv_nsec) / 1000;
    printf("Time: %f\n", (double)time/1000000);

    exit(0);
}


int main(void) {
  signal(SIGINT, sigint);

  // for 100% work
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    
  gtinit();		// initialize threads, see gthr.c
  gtgo(f);		// set f() as first thread
  gtgo(f);		// set f() as second thread
  gtgo(g);		// set g() as third thread
  gtgo(g);		// set g() as fourth thread
  gtret(1);		// wait until all threads terminate
}
