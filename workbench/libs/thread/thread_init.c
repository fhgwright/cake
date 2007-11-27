#include "thread_intern.h"

#include <exec/semaphores.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

static int GM_UNIQUENAME(Init)(struct ThreadBase *ThreadBase) {
    InitSemaphore(&ThreadBase->lock);

    NEWLIST(&ThreadBase->threads);

    return TRUE;
}

static int GM_UNIQUENAME(Expunge)(struct ThreadBase *ThreadBase) {
    return TRUE;
}

ADD2INITLIB(GM_UNIQUENAME(Init), 0)
ADD2EXPUNGELIB(GM_UNIQUENAME(Expunge), 0)
