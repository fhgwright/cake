#include "thread_intern.h"

#include <proto/exec.h>
#include <assert.h>

AROS_LH1(void, DestroyThreadCondition,
         AROS_LHA(ThreadCondition, cond, A0),
         struct ThreadBase *, ThreadBase, 15, Thread)
{
    AROS_LIBFUNC_INIT

    assert(cond != NULL);

    FreeMem(cond, sizeof(struct _ThreadCondition));

    AROS_LIBFUNC_EXIT
}
