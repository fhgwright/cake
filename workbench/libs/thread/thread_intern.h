#ifndef THREAD_INTERN_H
#define THREAD_INTERN_H 1

#define DEBUG 1
#include <aros/debug.h>

#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <stdint.h>

#include <libraries/thread.h>

typedef struct SignalSemaphore  *_Mutex;
typedef struct _ThreadCondition *_ThreadCondition;

struct _ThreadCondition {
    struct List     waiting;
    int             count;
};

struct _ThreadWaiter {
    struct Node     node;
    struct Task     *task;
};

struct ThreadBase {
    struct Library  library;
};

#endif
