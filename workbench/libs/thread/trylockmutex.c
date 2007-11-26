#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(BOOL, TryLockMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 12, Thread)
{
    AROS_LIBFUNC_INIT

    assert(mutex != NULL);

    return (BOOL) AttemptSemaphore((struct SignalSemaphore *) mutex);

    AROS_LIBFUNC_EXIT
}
