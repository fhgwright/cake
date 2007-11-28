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

static void entry_trampoline(void) {
    struct Library *aroscbase;
    struct Task *task = FindTask(NULL);
    struct trampoline_data *td = task->tc_UserData;
    struct ThreadBase *ThreadBase = td->ThreadBase;
    _Thread thread = &td->thread;
    void *result;
    BOOL detached;
    int exit_count;

    /* get the thread lock. we'll block here until CreateThread() releases the
     * lock before it exits */
    ObtainSemaphore(&thread->lock);
    ReleaseSemaphore(&thread->lock);

    aroscbase = OpenLibrary("arosc.library", 0);

    /* call the actual thread entry */
    result = AROS_UFC1(void *, td->entry,
                       AROS_UFCA(void *, td->data, A0));

    CloseLibrary(aroscbase);

    /* thread finished. save the result */
    ObtainSemaphore(&thread->lock);
    thread->result = result;

    /* we're done */
    thread->completed = TRUE;

    /* get data we need to figure out how to clean up */
    detached = thread->detached;
    exit_count = thread->exit_count;
    ReleaseSemaphore(&thread->lock);

    /* if we're detached or noone is waiting, the we have to cleanup ourselves */
    if (detached || exit_count == 0) {
        DestroyThreadCondition(thread->exit);
        DestroyMutex(thread->exit_mutex);
        FreeVec(td);
    }

    /* otherwise tell them, and they'll clean us up */
    else
        BroadcastThreadCondition(thread->exit);
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
}
