#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, LockMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 11, Thread)
{
    AROS_LIBFUNC_INIT

    assert(mutex != NULL);

    ObtainSemaphore((struct SignalSemaphore *) mutex);

    AROS_LIBFUNC_EXIT
}
