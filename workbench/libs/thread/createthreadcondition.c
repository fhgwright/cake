#include "thread_intern.h"

#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>

AROS_LH0(_ThreadCondition, CreateThreadCondition,
         struct ThreadBase *, ThreadBase, 14, Thread)
{
    AROS_LIBFUNC_INIT

    _ThreadCondition cond;

    if ((cond = AllocMem(sizeof(struct _ThreadCondition), MEMF_PUBLIC)) == NULL)
        return NULL;

    InitSemaphore(&cond->lock);
    NEWLIST(&cond->waiters);
    cond->count = 0;

    return cond;

    AROS_LIBFUNC_EXIT
}
