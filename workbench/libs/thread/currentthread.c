/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <proto/exec.h>

/*****************************************************************************

    NAME */
        AROS_LH0(ThreadIdentifier, CurrentThread,

/*  SYNOPSIS */

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 8, Thread)

/*  FUNCTION
        Get the ID of the running thread.

    INPUTS
        None.

    RESULT
        Numeric thread ID, or -1 if this is not a thread. 0 is a valid thread
        ID.

    NOTES

    EXAMPLE
        ThreadIdentifier id = CurrentThread();
        printf("this is thread %d\n", id);

    BUGS

    SEE ALSO
        CreateThread(), DetachThread(), WaitForThreadCompletion();

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* find it */
    _Thread thread = _getthreadbytask(FindTask(NULL), ThreadBase);
    if (thread == NULL)
        return -1;

    return thread->id;

    AROS_LIBFUNC_EXIT
} /* CurrentThread */
