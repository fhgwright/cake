#include "thread_intern.h"

#include <exec/semaphores.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

static int GM_UNIQUENAME(Open)(struct ThreadBase *ThreadBase) {
    InitSemaphore(&ThreadBase->lock);

    NEWLIST(&ThreadBase->threads);

    return TRUE;
}

static int GM_UNIQUENAME(Close)(struct ThreadBase *ThreadBase) {
    return TRUE;
}

ADD2OPENLIB(GM_UNIQUENAME(Open), 0)
ADD2CLOSELIB(GM_UNIQUENAME(Close), 0)
