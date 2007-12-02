/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */
        AROS_LH0(_Condition, CreateCondition,

/*  SYNOPSIS */

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 14, Thread)

/*  FUNCTION
        Creates a condition variable.

    INPUTS
        None.

    RESULT
        The newly created condition, or NULL if one couldn't be created.

    NOTES
        Condition cond = CreateCondition();

    EXAMPLE

    BUGS

    SEE ALSO
        DestroyCondition(), WaitCondition(), SignalCondition(),
        BroadcastCondition()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    _Condition cond;

    /* allocate */
    if ((cond = AllocMem(sizeof(struct _Condition), MEMF_PUBLIC)) == NULL)
        return NULL;

    /* initialise */
    InitSemaphore(&cond->lock);
    NEWLIST(&cond->waiters);
    cond->count = 0;

    return cond;

    AROS_LIBFUNC_EXIT
} /* CreateCondition */
