/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH1(BOOL, TryLockMutex,

/*  SYNOPSIS */
        AROS_LHA(_Mutex, mutex, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 12, Thread)

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

    assert(mutex != NULL);

    return (BOOL) AttemptSemaphore((struct SignalSemaphore *) mutex);

    AROS_LIBFUNC_EXIT
} /* TryLockMutex */
