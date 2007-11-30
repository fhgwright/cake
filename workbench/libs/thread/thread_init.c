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
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/thread.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

static int GM_UNIQUENAME(Open)(struct ThreadBase *ThreadBase) {
    InitSemaphore(&ThreadBase->lock);

    NEWLIST(&ThreadBase->threads);

    return TRUE;
}

static int GM_UNIQUENAME(Close)(struct ThreadBase *ThreadBase) {
    _Thread thread, next;

    /* detach any remaining threads. its hard to know what the right thing to
     * do here is, but we have to do something */
    ForeachNodeSafe(&ThreadBase->threads, thread, next)
        DetachThread(thread->id);

    return TRUE;
}

ADD2OPENLIB(GM_UNIQUENAME(Open), 0)
ADD2CLOSELIB(GM_UNIQUENAME(Close), 0)
