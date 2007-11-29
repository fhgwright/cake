#include "thread_intern.h"

#include <exec/semaphores.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(BOOL, DestroyMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 10, Thread)
{
    AROS_LIBFUNC_INIT

    assert(mutex != NULL);

    /* we can only destroy the mutex if its not held and noone is waiting */
    Forbid();
    if (((struct SignalSemaphore *) mutex)->ss_QueueCount >= 0) {
        Permit();
        return FALSE;
    }
    Permit();

    FreeMem(mutex, sizeof(struct SignalSemaphore));

    return TRUE;

    AROS_LIBFUNC_EXIT
}
