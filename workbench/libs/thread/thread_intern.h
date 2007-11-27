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

typedef struct _Thread          *_Thread;
typedef struct SignalSemaphore  *_Mutex;
typedef struct _ThreadCondition *_ThreadCondition;
typedef struct _ThreadWaiter    *_ThreadWaiter;

struct _Thread {
    ThreadIdentifier        id;
    ThreadFunction          entry;
    void                    *data;
    struct Task             *task;
};

struct _ThreadCondition {
    struct SignalSemaphore  lock;
    struct List             waiters;
    int                     count;
};

struct _ThreadWaiter {
    struct Node             node;
    struct Task             *task;
};

struct ThreadBase {
    struct Library          library;

    struct SignalSemaphore  lock;

    ThreadIdentifier        nextid;

    struct List             threads;
};

#endif
