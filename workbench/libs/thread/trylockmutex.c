#include "thread_intern.h"

AROS_LH1(BOOL, TryLockMutex,
         AROS_LHA(_Mutex, mutex, A0),
         struct ThreadBase *, ThreadBase, 12, Thread)
{
    AROS_LIBFUNC_INIT

    return FALSE;

    AROS_LIBFUNC_EXIT
}
