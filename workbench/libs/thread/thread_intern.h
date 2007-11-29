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

/* internal definitions of mutexes and conditions (they're void * in the
 * public header */
typedef struct SignalSemaphore  *_Mutex;
typedef struct _ThreadCondition *_ThreadCondition;

/* internal types */
typedef struct _Thread          *_Thread;
typedef struct _ThreadWaiter    *_ThreadWaiter;

/* a single thread */
struct _Thread {
    struct Node             node;       /* node for ThreadBase->threads */

    struct SignalSemaphore  lock;       /* lock for the this thread data */

    ThreadIdentifier        id;         /* numerical thread id. read only,
                                         * no need to acquire the lock */

    struct Task             *task;      /* the exec task for this thread */

    void                    *result;    /* storage for the thread exit value
                                         * for thread completion waiters */

    _ThreadCondition        exit;       /* condition for threads waiting for
                                         * this thread to finish */
    _Mutex                  exit_mutex; /* associated mutex */
    int                     exit_count; /* number of threads waitering */

    BOOL                    detached;   /* flag, thread is detached */
    BOOL                    completed;  /* flag, thread has completed */
};

/* a condition variable */
struct _ThreadCondition {
    struct SignalSemaphore  lock;       /* lock for this condition data */

    struct List             waiters;    /* list of _ThreadWaiters */
    int                     count;      /* number of waiters in the list */
};

/* a waiter for a condition */
struct _ThreadWaiter {
    struct Node             node;       /* node for cond->waiters */
    struct Task             *task;      /* task to signal when the condition
                                         * is met */
};

/* the library base. this is a per-opener base */
struct ThreadBase {
    struct Library          library;

    struct ThreadBase       *rootbase;  /* pointer to the global base */

    struct SignalSemaphore  lock;       /* lock for this base */

    ThreadIdentifier        nextid;     /* numeric identifier to be issued to
                                         * the next thread created */

    struct List             threads;    /* list of threads */
};

/* helper functions for finding thread data */
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
