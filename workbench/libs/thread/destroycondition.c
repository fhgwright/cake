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
#include <proto/exec.h>
#include <assert.h>

/*****************************************************************************

    NAME */
        AROS_LH1(BOOL, DestroyCondition,

/*  SYNOPSIS */
        AROS_LHA(_Condition, cond, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 15, Thread)

/*  FUNCTION
        Destroys a condition variable.

    INPUTS
        cond - the condition variable to destroy.

    RESULT
        TRUE if the condition was destroyed, otherwise FALSE.

    NOTES
        You cannot destroy a condition variable if other threads are waiting on
        it.

    EXAMPLE
        DestroyCondition(cond);

    BUGS

    SEE ALSO
        CreateCondition(), WaitCondition(), SignalCondition(),
        BroadcastCondition()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    assert(cond != NULL);

    /* we can only destroy the cond if noone is waiting on it */
    ObtainSemaphoreShared(&cond->lock);
    if (cond->count > 0) {
        ReleaseSemaphore(&cond->lock);
        return FALSE;
    }

    FreeMem(cond, sizeof(struct _Condition));

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* DestroyCondition */
