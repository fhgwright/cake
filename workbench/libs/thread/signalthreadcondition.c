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
        AROS_LH1(void, SignalThreadCondition,

/*  SYNOPSIS */
        AROS_LHA(_ThreadCondition, cond, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 17, Thread)

/*  FUNCTION
        Signals a thread waiting on condition variable.

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

        If no threads are waiting on the condition, nothing happens. If more
        than one thread is waiting, only one will be signalled. Which one is
        undefined.

    EXAMPLE
        LockMutex(mutex);
        SignalThreadCondition(cond);
        UnlockMutex(mutex);

    BUGS

    SEE ALSO
        CreateThreadCondition(), DestroyThreadCondition(),
        WaitForThreadCondition(), BroadcastThreadCondition()

    INTERNALS
        SIGF_SIGNAL is used to signal the selected waiting thread.

*****************************************************************************/
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
} /* SignalThreadCondition */
