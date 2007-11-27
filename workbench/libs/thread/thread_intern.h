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
    struct Node             node;

    struct SignalSemaphore  lock;

    ThreadIdentifier        id;

    struct Task             *task;

    void                    *result;

    _ThreadCondition        exit;
    _Mutex                  exit_mutex;
    int                     exit_count;

    BOOL                    detached;
    BOOL                    completed;
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

static inline _Thread _getthreadbyid(ThreadIdentifier id, struct ThreadBase *ThreadBase) {
    _Thread thread, next;
    ForeachNodeSafe(&ThreadBase->threads, thread, next) {
        if (thread->id == id)
            return thread;
    }
    return NULL;
}

static inline _Thread _getthreadbytask(struct Task *task, struct ThreadBase *ThreadBase) {
    _Thread thread, next;
    ForeachNodeSafe(&ThreadBase->threads, thread, next) {
        if (thread->task == task)
            return thread;
    }
    return NULL;
}

#endif
