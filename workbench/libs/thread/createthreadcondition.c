#include "thread_intern.h"

#include <exec/memory.h>
#include <proto/exec.h>

AROS_LH0(_ThreadCondition, CreateThreadCondition,
         struct ThreadBase *, ThreadBase, 14, Thread)
{
    AROS_LIBFUNC_INIT

    _ThreadCondition *cond;

    if ((cond = AllocMem(sizeof(struct _ThreadCondition), MEMF_PUBLIC | MEMF_CLEAR)) == NULL)
        return NULL;

    return cond;

    AROS_LIBFUNC_EXIT
}
