#include "thread_intern.h"

#include <exec/types.h>
#include <proto/exec.h>
#include <assert.h>

AROS_LH1(BOOL, DestroyThreadCondition,
         AROS_LHA(_ThreadCondition, cond, A0),
         struct ThreadBase *, ThreadBase, 15, Thread)
{
    AROS_LIBFUNC_INIT

    BOOL in_use = FALSE;

    assert(cond != NULL);

    /* we can only destroy the cond if its not held and noone is waiting */
    ObtainSemaphoreShared(&cond->lock);
    if (cond->count > 0)
        in_use = TRUE;
    ReleaseSemaphore(&cond->lock);

    if (in_use)
        return FALSE;

    FreeMem(cond, sizeof(struct _ThreadCondition));

    return TRUE;

    AROS_LIBFUNC_EXIT
}
