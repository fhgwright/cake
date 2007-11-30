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
