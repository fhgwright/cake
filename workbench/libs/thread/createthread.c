#include "thread_intern.h"

AROS_LH2(ThreadIdentifier, CreateThread,
         AROS_LHA(ThreadFunction, entry_point, A0),
         AROS_LHA(void *,         data,        A1),
         struct ThreadBase *, ThreadBase, 5, Thread)
{
    AROS_LIBFUNC_INIT

    return 0;

    AROS_LIBFUNC_EXIT
}
