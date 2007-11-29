#include "thread_intern.h"

#include <proto/exec.h>

AROS_LH0(ThreadIdentifier, CurrentThread,
         struct ThreadBase *, ThreadBase, 8, Thread)
{
    AROS_LIBFUNC_INIT

    /* find it */
    _Thread thread = _getthreadbytask(FindTask(NULL), ThreadBase);
    if (thread == NULL)
        return -1;

    return thread->id;

    AROS_LIBFUNC_EXIT
}
