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
        AROS_LH1(BOOL, DestroyThreadCondition,

/*  SYNOPSIS */
        AROS_LHA(_ThreadCondition, cond, A0),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 15, Thread)

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

    assert(cond != NULL);

    /* we can only destroy the cond if its not held and noone is waiting */
    ObtainSemaphoreShared(&cond->lock);
    if (cond->count > 0) {
        ReleaseSemaphore(&cond->lock);
        return FALSE;
    }

    FreeMem(cond, sizeof(struct _ThreadCondition));

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* DestroyThreadCondition */
