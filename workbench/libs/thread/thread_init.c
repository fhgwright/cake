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

    /* XXX we're most likely here because main() exited. if there are
     * remaining threads, we need to do something with them. we have the
     * following options:
     * 
     * 1. leave them alone
     * 2. wait for them to finish
     * 3. kill them
     *
     * [1] is problematic because entry_trampoline() in createthread.c expects
     * ThreadBase to be valid after the user thread function completes so it
     * can do cleanup. ThreadBase will be invalid not long after this Close
     * function completes, so entry_trampoline() will try to get the base lock
     * and will fail and crash, usually taking AROS down with it.
     * 
     * [2] is fine, but there's no guarantee that they ever will finish;
     * furthermore this Close function is inside Forbid() right now, which
     * means we'd have to re-enable task switches. that's safe because this is
     * a per-opener base, but its just a little bit tricky. also we need an
     * extra condition variable and counter to watch all thread exits at once.
     *
     * [3] ensures that the threads are gone and the main task can exit right
     * now, but AROS really doesn't provide a way to safely kill a process.
     * RemTask() will make sure it never gets scheduled again and will free
     * the memory it allocated, but it may have open libraries, filehandles,
     * etc which will get leaked. This can't be fixed without proper task
     * resource tracking.
     *
     * I've chosen [3] for now, because it seems to make the most sense in that
     * once main exits, the process should no longer exists, but in reality
     * they all suck. for now, the user should ensure that all the threads
     * have completed before the main task exits.
     */

    ListLength(&ThreadBase->threads, count);
    if (count > 0) {
        kprintf("[thread] %d threads still running, killing them\n", count);

        ForeachNode(&ThreadBase->threads, thread) {
            RemTask(thread->task);

            kprintf("  %d (task 0x%p) killed\n", thread->id, thread->task);

            FreeVec(thread);
        }

        kprintf("[thread] system may be unstable! reboot as soon as possible\n");
    }

    return TRUE;
}

ADD2OPENLIB(GM_UNIQUENAME(Open), 0)
ADD2CLOSELIB(GM_UNIQUENAME(Close), 0)
