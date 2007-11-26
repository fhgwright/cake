#include "thread_intern.h"

#include <exec/types.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(BOOL, DestroyThreadCondition,
         AROS_LHA(ThreadCondition, cond, A0),
         struct ThreadBase *, ThreadBase, 15, Thread)
{
    AROS_LIBFUNC_INIT

    assert(cond != NULL);

    /* XXX make sure noone is waiting */

    FreeMem(cond, sizeof(struct _ThreadCondition));

    return TRUE;

    AROS_LIBFUNC_EXIT
}
