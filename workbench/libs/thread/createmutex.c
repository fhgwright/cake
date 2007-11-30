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
#include <exec/memory.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */
        AROS_LH0(_Mutex, CreateMutex,

/*  SYNOPSIS */

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 9, Thread)

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

    struct SignalSemaphore *sem;

    if ((sem = (struct SignalSemaphore *) AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC)) == NULL)
        return NULL;

    InitSemaphore(sem);

    return (_Mutex) sem;

    AROS_LIBFUNC_EXIT
} /* CreateMutex */
