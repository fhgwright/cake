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

    /* find it */
    _Thread thread = _getthreadbytask(FindTask(NULL), ThreadBase);
    if (thread == NULL)
        return -1;

    return thread->id;

    AROS_LIBFUNC_EXIT
} /* CurrentThread */
