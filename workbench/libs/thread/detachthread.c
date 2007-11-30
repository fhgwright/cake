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

    EXAMPLE
        DetachThread(id);

    BUGS
        The semantics around detached threads are not clear. Under other thread
        systems a detached thread can complete and cleanup its own resources,
        whereas a non-detached thread will continue to exist until it is asked
        to cleanup explicitly (ie via WaitForThreadCompletion()).

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

    ObtainSemaphore(&thread->lock);

    /* can't detach if its already detached */
    if (thread->detached) {
        ReleaseSemaphore(&thread->lock);
        return FALSE;
    }

    /* simple */
    thread->detached = TRUE;
    
    ReleaseSemaphore(&thread->lock);

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* DetachThread */
