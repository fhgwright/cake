/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright � 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/semaphores.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

static int GM_UNIQUENAME(Open)(struct ThreadBase *ThreadBase) {
    InitSemaphore(&ThreadBase->lock);

    NEWLIST(&ThreadBase->threads);

    return TRUE;
}

static int GM_UNIQUENAME(Close)(struct ThreadBase *ThreadBase) {
    int count;
    _Thread thread;

    ListLength(&ThreadBase->threads, count);
    if (count > 0) {
        kprintf("[thread] %d threads still running, killing them\n", count);

        ForeachNode(&ThreadBase->threads, thread) {
            RemTask(thread->task);

            kprintf("  %d (task 0x%p) killed\n", thread->id, thread->task);

            FreeVec(thread);
        }
    }

    return TRUE;
}

ADD2OPENLIB(GM_UNIQUENAME(Open), 0)
ADD2CLOSELIB(GM_UNIQUENAME(Close), 0)
