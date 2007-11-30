/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/types.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <proto/thread.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH2(BOOL, WaitForThreadCompletion,

/*  SYNOPSIS */
        AROS_LHA(ThreadIdentifier, thread_id, D0),
        AROS_LHA(void **,          result,    A1),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 6, Thread)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    _Thread thread;

    assert(thread_id >= 0);

    /* get the thread */
    if ((thread = _getthreadbyid(thread_id, ThreadBase)) == NULL)
        return FALSE;

    /* can't wait for ourselves, that would be silly */
    if (thread->task == FindTask(NULL))
        return FALSE;

    ObtainSemaphore(&thread->lock);

    /* can't wait on a detached thread, or one thats already finished */
    if (thread->detached || thread->completed) {
        ReleaseSemaphore(&thread->lock);
        return FALSE;
    }

    /* one more waiter */
    thread->exit_count++;
    ReleaseSemaphore(&thread->lock);

    /* wait for exit */
    LockMutex(thread->exit_mutex);
    WaitForThreadCondition(thread->exit, thread->exit_mutex);
    UnlockMutex(thread->exit_mutex);

    ObtainSemaphore(&thread->lock);

    /* copy the result if the caller was interested */
    if (result != NULL)
        *result = thread->result;

    /* no longer waiting */
    thread->exit_count--;

    /* still more threads waiting, so we're done */
    if (thread->exit_count > 0) {
        ReleaseSemaphore(&thread->lock);
        return TRUE;
    }

    /* nobody else cares about this thread, so it can be cleaned up */
    ObtainSemaphore(&ThreadBase->lock);
    REMOVE(thread);
    ReleaseSemaphore(&ThreadBase->lock);

    DestroyThreadCondition(thread->exit);
    DestroyMutex(thread->exit_mutex);
    FreeVec(thread);

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* WaitForThreadCompletion */
