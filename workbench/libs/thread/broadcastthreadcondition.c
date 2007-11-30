/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH1(void, BroadcastThreadCondition,

/*  SYNOPSIS */
        AROS_LHA(_ThreadCondition, cond, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 18, Thread)

/*  FUNCTION
        Signals all threads waiting on a condition variable.

    INPUTS
        cond - the condition to signal.

    RESULT
        This function always succeeds.

    NOTES
        Before calling this function you should lock the mutex that protects
        the condition. WaitForThreadCondition() atomically unlocks the mutex
        and waits on the condition, so by locking the mutex first before
        sending the signal, you ensure that the signal cannot be missed. See
        WaitForThreadCondition() for more details.

        If no threads are waiting on the condition, nothing happens.

    EXAMPLE
        LockMutex(mutex);
        BroadcastThreadCondition(cond);
        UnlockMutex(mutex);

    BUGS

    SEE ALSO
        CreateThreadCondition(), DestroyThreadCondition(),
        WaitForThreadCondition(), SignalThreadCondition()

    INTERNALS
        SIGF_SIGNAL is used to signal the selected waiting thread.

*****************************************************************************/
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
} /* BroadcastThreadCondition */
