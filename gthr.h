enum {
    MaxGThreads = 5,		// Maximum number of threads, used as array size for gttbl
    StackSize = 0x400000,	// Size of stack of each thread
};

struct gt {
  // Saved context, switched by gtswtch.S (see for detail)
  struct gtctx {
    uint64_t rsp;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
  }
  ctx;
  // Thread state
  enum {
    Unused,
    Running,
    Ready,
  }
  st;
  
  int priority;
  int actualPriority;
  int tickets[2];
  
  struct timespec startRunClock, endRunClock;
  double totalRunTime;
  double minRunTime;
  double maxRunTime;
  int countRunAvg;
  
  struct timespec startWaitClock, endWaitClock;
  double totalWaitTime;
  double minWaitTime;
  double maxWaitTime;
  int countWaitAvg;
};

void gtinit(int type, int priority, int a, int b);				// initialize gttbl
void gtret(int ret);			// terminate thread
void gtswtch(struct gtctx * old, struct gtctx * new);	// declaration from gtswtch.S
bool gtyield(void);				// yield and switch to another thread
void gtstop(void);				// terminate current thread
int gtgo(void( * f)(void), int priority, int a, int b);		// create new thread and set f as new "run" function
void resetsig(int sig);			// reset signal
void gthandle(int sig);			// periodically triggered by alarm
int uninterruptibleNanoSleep(time_t sec, long nanosec);	// uninterruptible sleep
