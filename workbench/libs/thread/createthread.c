/*
 * thread.library - threading and synchronisation primitives
 *
 * Copyright © 2007 Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 */

#include "thread_intern.h"

#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/thread.h>
#include <assert.h>

struct trampoline_data {
    struct _Thread      thread;
    struct ThreadBase   *ThreadBase;
    ThreadFunction      entry;
    void                *data;
};

static void entry_trampoline(void);

/*****************************************************************************

    NAME */
        AROS_LH2(ThreadIdentifier, CreateThread,

/*  SYNOPSIS */
        AROS_LHA(ThreadFunction, entry, A0),
        AROS_LHA(void *,         data,  A1),

/*  LOCATION */
        struct ThreadBase *, ThreadBase, 5, Thread)

/*  FUNCTION
        Creates a new thread.

    INPUTS
        entry - pointer to a function to run in the new thread
        data  - pointer to pass in the first in the first argument to function
                pointed to by entry

    RESULT
        Numeric thread ID, or -1 if thread could not be started. 0 is a valid
        thread ID.

    NOTES

    EXAMPLE
        ThreadIdentifier id = CreateThread(entry, data);
        if (id < 0)
            printf("thread creation failed\n");
        else
            printf("thread %d created\n", id);

    BUGS

    SEE ALSO
        CurrentThread(), DetachThread(), WaitForThreadCompletion()

    INTERNALS
        Each thread gets its own instance of arosc.library, so it can safely
        call functions like printf() without conflicting with other threads.
        Similarly, each thread gets its own standard I/O streams, though they
        start attached to the same place as the task that created the thread.

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct trampoline_data *td;
    _Thread thread;
    ThreadIdentifier id;

    assert(entry != NULL);

    /* allocate some space for the thread and stuff the trampoline needs */
    if ((td = AllocVec(sizeof(struct trampoline_data), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
        return -1;

    thread = &td->thread;
    td->ThreadBase = ThreadBase;

    /* entry point info for the trampoline */
    td->entry = entry;
    td->data = data;

    /* setup the lock */
    InitSemaphore(&thread->lock);
    ObtainSemaphore(&thread->lock);

    /* create the new process and hand control to the trampoline. it will wait
     * for us to finish setting up because we have the thread lock */
    thread->task = (struct Task *) CreateNewProcTags(
        NP_Name,        (IPTR) "thread.library thread",
        NP_Entry,       (IPTR) entry_trampoline,
        NP_UserData,    (IPTR) td,
        NP_Input,       (IPTR) OpenFromLock(DupLockFromFH(Input())),
        NP_Output,      (IPTR) OpenFromLock(DupLockFromFH(Output())),
        NP_Error,       (IPTR) OpenFromLock(DupLockFromFH(Error())),
        TAG_DONE);

    /* failure, shut it down */
    if (thread->task == NULL) {
        FreeMem(td, sizeof(struct trampoline_data));
        return -1;
    }

    /* make a condition so other threads can wait for us to exit */
    thread->exit = CreateThreadCondition();
    thread->exit_mutex = CreateMutex();

    ObtainSemaphore(&ThreadBase->lock);

    /* get an id */
    id = thread->id = ThreadBase->nextid++;

    /* add the thread to the list */
    ADDTAIL(&ThreadBase->threads, thread);

    ReleaseSemaphore(&ThreadBase->lock);

    /* unlock the thread to kick it off */
    ReleaseSemaphore(&thread->lock);

    return id;

    AROS_LIBFUNC_EXIT
} /* CreateThread */

static void entry_trampoline(void) {
    struct Library *aroscbase;
    struct Task *task = FindTask(NULL);
    struct trampoline_data *td = task->tc_UserData;
    struct ThreadBase *ThreadBase = td->ThreadBase;
    _Thread thread = &td->thread;
    void *result;

    /* get the thread lock. we'll block here until CreateThread() releases the
     * lock before it exits */
    ObtainSemaphore(&thread->lock);
    ReleaseSemaphore(&thread->lock);

    /* give each thread its own C library, so it can reliably printf() etc */
    aroscbase = OpenLibrary("arosc.library", 0);

    /* call the actual thread entry */
    result = AROS_UFC1(void *, td->entry,
                       AROS_UFCA(void *, td->data, A0));

    CloseLibrary(aroscbase);

    /* its over, update its state */
    ObtainSemaphore(&thread->lock);

    /* if its detached, then we close it down right here and now */
    if (thread->detached) {
        DestroyThreadCondition(thread->exit);
        DestroyMutex(thread->exit_mutex);
        FreeVec(td);
        
        return;
    }

    /* save the result */
    thread->result = result;

    /* mark it as done */
    thread->completed = TRUE;

    ReleaseSemaphore(&thread->lock);

    /* tell anyone that cares. we'll be cleaned up in
     * WaitForThreadCompletion() */
    LockMutex(thread->exit_mutex);
    BroadcastThreadCondition(thread->exit);
    UnlockMutex(thread->exit_mutex);
}
