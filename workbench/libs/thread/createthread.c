#include "thread_intern.h"

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

static void entry_trampoline(void) {
    struct Task *task = FindTask(NULL);
    struct trampoline_data *td = task->tc_UserData;
    struct ThreadBase *ThreadBase = td->ThreadBase;
    _Thread thread = &td->thread;
    void *result;
    BOOL detached;

    /* get the thread lock. we'll block here until CreateThread() releases the
     * lock before it exits */
    ObtainSemaphore(&thread->lock);
    ReleaseSemaphore(&thread->lock);

    /* call the actual thread entry */
    result = AROS_UFC1(void *, td->entry,
                       AROS_UFCA(void *, td->data, A0));

    /* thread finished. find out if it was detached */
    ObtainSemaphoreShared(&thread->lock);
    detached = thread->detached;
    ReleaseSemaphore(&thread->lock);

    /* let anyone waiting for us know that we're gone */
    BroadcastThreadCondition(thread->exit);

    /* all done, cleanup and we're outta here */
    FreeMem(thread, sizeof(struct _Thread));
}

AROS_LH2(ThreadIdentifier, CreateThread,
         AROS_LHA(ThreadFunction, entry, A0),
         AROS_LHA(void *,         data,  A1),
         struct ThreadBase *, ThreadBase, 5, Thread)
{
    AROS_LIBFUNC_INIT

    struct trampoline_data *td;
    _Thread thread;
    ThreadIdentifier id;

    assert(entry != NULL);

    /* allocate some space for the thread and stuff the trampoline needs */
    if ((td = AllocMem(sizeof(struct trampoline_data), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
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
        TAG_DONE);

    /* failure, shut it down */
    if (thread->task == NULL) {
        FreeMem(td, sizeof(struct trampoline_data));
        return -1;
    }

    ObtainSemaphore(&ThreadBase->lock);

    /* get an id */
    id = thread->id = ThreadBase->nextid++;

    /* add the thread to the list */
    ADDTAIL(&ThreadBase->threads, thread);

    ReleaseSemaphore(&ThreadBase->lock);

    /* make a condition so other threads can wait for us to exit */
    thread->exit = CreateThreadCondition();

    /* unlock the thread to kick it off */
    ReleaseSemaphore(&thread->lock);

    return id;

    AROS_LIBFUNC_EXIT
}
