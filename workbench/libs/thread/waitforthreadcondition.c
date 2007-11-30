/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright � 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/thread.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH2(BOOL, WaitForThreadCondition,

/*  SYNOPSIS */
        AROS_LHA(_ThreadCondition, cond,  A0),
        AROS_LHA(_Mutex,           mutex, A1),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 16, Thread)

/*  FUNCTION
        Blocks until a condition is signaled.

    INPUTS
        cond - the condition variable to wait on.
        mutex - a mutex that protects the condition

    RESULT
        TRUE if the condition was signaled, FALSE if an error occured.

    NOTES
        This function will atomically unlock the mutex and wait on the
        condition. The thread is suspended until the condition is signalled.
        After the condition is signalled, the mutex is relocked before
        returning to the caller.

        The use of a mutex in conjunction with waiting on and signalling the
        condition ensures that no signals are missed. See
        SignalThreadCondition() for more details.

    EXAMPLE
        LockMutex(mutex);
        WaitForThreadCondition(cond, mutex);
        UnlockMutex(mutex);

    BUGS

    SEE ALSO
        CreateThreadCondition(), DestroyThreadCondition(),
        SignalThreadCondition(), BroadcastThreadCondition()

    INTERNALS
        Waiting on a condition causes the current thread to wait to receive
        SIGF_SINGLE from the signalling task.

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    _ThreadWaiter waiter;

    assert(cond != NULL);
    assert(mutex != NULL);

    /* setup a new waiter */
    if ((waiter = AllocMem(sizeof(struct _ThreadWaiter), MEMF_CLEAR)) == NULL) {
        return FALSE;
    }
    waiter->task = FindTask(NULL);

    /* safely add ourselves to the list of waiters */
    ObtainSemaphore(&cond->lock);
    ADDTAIL(&cond->waiters, waiter);
    cond->count++;
    ReleaseSemaphore(&cond->lock);

    /* disable task switches. we must atomically unlock the mutex and wait for
     * the signal, otherwise the signal may be missed */
    Forbid();

    /* release the mutex that protects the condition */
    UnlockMutex(mutex);

    /* and now wait for someone to hit the condition. this will break the
     * Forbid(), which is what we want */
    Wait(SIGF_SINGLE);

    /* the Forbid() is restored when Wait() exits, so we have to turn task
     * switches on again. */
    Permit();

    /* retake the mutex */
    LockMutex(mutex);

    /* done. note that we're not removing ourselves from the list of waiters,
     * that has been done by the signalling task */
    return TRUE;

    AROS_LIBFUNC_EXIT
} /* WaitForThreadCondition */
