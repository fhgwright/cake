#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/thread.h>
#include <assert.h>

AROS_LH2(void, WaitForThreadCondition,
         AROS_LHA(_ThreadCondition, cond, A0),
         AROS_LHA(_Mutex,           mutex,     A1),
         struct ThreadBase *, ThreadBase, 16, Thread)
{
    AROS_LIBFUNC_INIT

    _ThreadWaiter waiter;

    assert(cond != NULL);
    assert(mutex != NULL);

    /* setup a new waiter */
    if ((waiter = AllocMem(sizeof(struct _ThreadWaiter), MEMF_CLEAR)) == NULL) {
        /* XXX return failure */
        return;
    }
    waiter->task = FindTask(NULL);

    /* safely add ourselves to the list of waiters */
    ObtainSemaphore(&cond->lock);
    ADDTAIL(&cond->waiters, waiter);
    cond->count++;
    ReleaseSemaphore(&cond->lock);

    /* release the mutex that protects the condition */
    UnlockMutex(mutex);

    /* and now wait for someone to hit the condition */
    Wait(SIGF_SINGLE);

    /* condition signaled, retake the mutex */
    LockMutex(mutex);

    /* done. note that we're not removing ourselves from the list of waiters,
     * that has been done by the signalling task */

    AROS_LIBFUNC_EXIT
}
