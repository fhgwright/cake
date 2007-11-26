#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, SignalThreadCondition,
         AROS_LHA(_ThreadCondition, cond, A0),
         struct ThreadBase *, ThreadBase, 17, Thread)
{
    AROS_LIBFUNC_INIT

    _ThreadWaiter waiter;

    assert(cond != NULL);

    /* safely remove a waiter from the list */
    ObtainSemaphore(&cond->lock);
    waiter = (_ThreadWaiter) REMHEAD(&cond->waiters);
    if (waiter != NULL)
        cond->count--;
    ReleaseSemaphore(&cond->lock);

    /* noone waiting */
    if (waiter == NULL)
        return;

    /* signal the task */
    Signal(waiter->task, SIGF_SINGLE);

    /* all done */
    FreeMem(waiter, sizeof(struct _ThreadWaiter));

    AROS_LIBFUNC_EXIT
}
