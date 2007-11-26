#include "thread_intern.h"

AROS_LH2(int, WaitForThreadCompletion,
         AROS_LHA(ThreadIdentifier, thread_id, D0),
         AROS_LHA(void **,          result,    A1),
         struct ThreadBase *, ThreadBase, 6, Thread)
{
    AROS_LIBFUNC_INIT

    return 0;

    AROS_LIBFUNC_EXIT
}
