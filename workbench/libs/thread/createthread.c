#include "thread_intern.h"

#include <exec/tasks.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <assert.h>

static void entry_trampoline(void) {
    struct Task *task = FindTask(NULL);
    _Thread thread = task->tc_UserData;
    void *result;
    BOOL detached;

    /* get the thread lock. we'll block here until CreateThread() releases the
     * lock before it exits */
    ObtainSemaphore(&thread->lock);
    ReleaseSemaphore(&thread->lock);

    /* call the actual thread entry */
    result = AROS_UFC1(void *, thread->entry,
                       AROS_UFCA(void *, thread->data, A0));

    /* thread finished. find out if it was detached */
    ObtainSemaphoreShared(&thread->lock);
    detached = thread->detached;
    ReleaseSemaphore(&thread->lock);

    /* it wasn't, so wait for someone to join us */
    if (!detached)
        Wait(SIGF_SINGLE);

    /* all done, cleanup and we're outta here */
    FreeMem(thread, sizeof(struct _Thread));
}

AROS_LH2(ThreadIdentifier, CreateThread,
         AROS_LHA(ThreadFunction, entry, A0),
         AROS_LHA(void *,         data,  A1),
         struct ThreadBase *, ThreadBase, 5, Thread)
{
    AROS_LIBFUNC_INIT

    _Thread thread;
    ThreadIdentifier id;

    assert(entry != NULL);

    /* make a new thread */
    if ((thread = AllocMem(sizeof(struct _Thread), MEMF_PUBLIC)) == NULL)
        return -1;

    /* setup the lock */
    InitSemaphore(&thread->lock);
    ObtainSemaphore(&thread->lock);

    /* get an id */
    ObtainSemaphore(&ThreadBase->lock);
    id = thread->id = ThreadBase->nextid++;
    ReleaseSemaphore(&ThreadBase->lock);

    /* entry point info for the trampoline */
    thread->entry = entry;
    thread->data = data;

    /* create the new process and hand control to the trampoline */
    thread->task = (struct Task *) CreateNewProcTags(
        NP_Name,        (IPTR) "thread.library thread",
        NP_Entry,       (IPTR) entry_trampoline,
        NP_UserData,    (IPTR) thread,
        TAG_DONE);

    /* failure, shut it down */
    if (thread->task == NULL) {
        FreeMem(thread, sizeof(struct _Thread));
        return -1;
    }

    /* unlock the thread to send it on its way */
    ReleaseSemaphore(&thread->lock);

    return id;

    AROS_LIBFUNC_EXIT
}
