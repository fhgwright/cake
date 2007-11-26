#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, BroadcastThreadCondition,
         AROS_LHA(_ThreadCondition, cond, A0),
         struct ThreadBase *, ThreadBase, 18, Thread)
{
    AROS_LIBFUNC_INIT

    _ThreadWaiter waiter;

    assert(cond != NULL);

    /* safely operation on the condition */
    ObtainSemaphore(&cond->lock);

    /* wake up all the waiters */
    while ((waiter = (_ThreadWaiter) REMHEAD(&cond->waiters)) != NULL) {
        Signal(waiter->task, SIGF_SINGLE);
        FreeMem(waiter, sizeof(struct _ThreadWaiter));
    }

    /* none left */
    cond->count = 0;

    ReleaseSemaphore(&cond->lock);

    AROS_LIBFUNC_EXIT
}
