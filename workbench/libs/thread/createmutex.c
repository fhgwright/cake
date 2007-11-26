#include "thread_intern.h"

#include <exec/semaphores.h>
#include <exec/memory.h>
#include <proto/exec.h>

AROS_LH0(_Mutex, CreateMutex,
         struct ThreadBase *, ThreadBase, 9, Thread)
{
    AROS_LIBFUNC_INIT

    struct SignalSemaphore *sem;

    if ((sem = (struct SignalSemaphore *) AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC)) == NULL)
        return NULL;

    InitSemaphore(sem);

    return (_Mutex) sem;

    AROS_LIBFUNC_EXIT
}
