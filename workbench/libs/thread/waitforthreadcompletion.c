#include "thread_intern.h"

#include <exec/types.h>
#include <exec/tasks.h>
#include <proto/exec.h>
#include <proto/thread.h>
#include <assert.h>

AROS_LH2(BOOL, WaitForThreadCompletion,
         AROS_LHA(ThreadIdentifier, thread_id, D0),
         AROS_LHA(void **,          result,    A1),
         struct ThreadBase *, ThreadBase, 6, Thread)
{
    AROS_LIBFUNC_INIT

    _Thread thread;
    BOOL detached, completed;

    assert(thread_id >= 0);

    if ((thread = _getthreadbyid(thread_id, ThreadBase)) == NULL)
        return FALSE;

    if (thread->task == FindTask(NULL))
        return FALSE;

    ObtainSemaphore(&thread->lock);
    detached = thread->detached;
    completed = thread->completed;

    if (detached || completed) {
        ReleaseSemaphore(&thread->lock);
        return FALSE;
    }

    thread->exit_count++;
    ReleaseSemaphore(&thread->lock);

    LockMutex(thread->exit_mutex);
    WaitForThreadCondition(thread->exit, thread->exit_mutex);
    UnlockMutex(thread->exit_mutex);

    ObtainSemaphore(&thread->lock);
    if (result != NULL)
        *result = thread->result;
    thread->exit_count--;

    if (thread->exit_count > 0)
        ReleaseSemaphore(&thread->lock);

    else {
        DestroyThreadCondition(thread->exit);
        DestroyMutex(thread->exit_mutex);
        FreeVec(thread);
    }

    return TRUE;

    AROS_LIBFUNC_EXIT
}
