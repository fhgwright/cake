#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, DestroyMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 10, Thread)
{
    AROS_LIBFUNC_INIT

    assert(mutex != NULL);

    FreeMem(mutex, sizeof(struct SignalSemaphore));

    AROS_LIBFUNC_EXIT
}
