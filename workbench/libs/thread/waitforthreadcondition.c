#include "thread_intern.h"

AROS_LH2(void, WaitForThreadCondition,
         AROS_LHA(_ThreadCondition, cond, A0),
         AROS_LHA(_Mutex,           mutex,     A1),
         struct ThreadBase *, ThreadBase, 16, Thread)
{
    AROS_LIBFUNC_INIT



    AROS_LIBFUNC_EXIT
}
