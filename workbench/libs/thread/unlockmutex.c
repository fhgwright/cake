#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, UnlockMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 13, Thread)
{
    AROS_LIBFUNC_INIT

    assert(mutex != NULL);

    ReleaseSemaphore((struct SignalSemaphore *) mutex);

    AROS_LIBFUNC_EXIT
}
