#include "thread_intern.h"

AROS_LH1(BOOL, DetachThread,
         AROS_LHA(ThreadIdentifier, thread_id, D0),
         struct ThreadBase *, ThreadBase, 7, Thread)
{
    AROS_LIBFUNC_INIT

    _Thread thread = _getthreadbyid(thread_id, ThreadBase);
    if (thread == NULL)
        return FALSE;

    ObtainSemaphore(&thread->lock);
    if (thread->detached) {
        ReleaseSemaphore(&thread->lock);
        return FALSE;
    }
    thread->detached = TRUE;
    ReleaseSemaphore(&thread->lock);

    return TRUE;

    AROS_LIBFUNC_EXIT
}
