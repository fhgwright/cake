/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

/*****************************************************************************

    NAME */
        AROS_LH1(BOOL, DetachThread,

/*  SYNOPSIS */
        AROS_LHA(ThreadIdentifier, thread_id, D0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 7, Thread)

/*  FUNCTION
        Detaches a thread from the parent process.

    INPUTS
        thread_id - ID of thread to detach.

    RESULT
        TRUE if the thread was detached, FALSE if the thread was already
        detached or another error occured.

    NOTES
        You cannot detach a thread that is already detached.

        This effectively turns the thread into a first-class process. It will
        survive even after the other threads and main task are gone, unless it
        exits.
        
        Once detached, the thread is no longer accessible from any other
        thread. Additionally, resources that were available to the thread
        previously (eg libraries) may no longer be available. This includes
        thread.library itself! The thread should explicitly open anything it
        needs.

    EXAMPLE
        DetachThread(id);

    BUGS

    SEE ALSO
        CreateThread(), CurrentThread(), WaitForThreadCompletion()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* get thread data */
    _Thread thread = _getthreadbyid(thread_id, ThreadBase);
    if (thread == NULL)
        return FALSE;

    /* mark it detached */
    ObtainSemaphore(&thread->lock);
    thread->detached = TRUE;
    ReleaseSemaphore(&thread->lock);

    /* remove it from the thread list too */
    ObtainSemaphore(&ThreadBase->lock);
    REMOVE(thread);
    ReleaseSemaphore(&ThreadBase->lock);

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* DetachThread */
