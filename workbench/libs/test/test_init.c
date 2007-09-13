#include <exec/types.h>
#include <exec/resident.h>
#include <proto/exec.h>

#include LC_LIBDEFS_FILE

#include <aros/symbolsets.h>
#include "test_intern.h"


static int GM_UNIQUENAME(Init)(LIBBASETYPEPTR tb) {
    return TRUE;
}

static int GM_UNIQUENAME(Expunge)(LIBBASETYPEPTR tb) {
    return TRUE;
}

/*
ADD2INITLIB(GM_UNIQUENAME(Init), 0)
ADD2EXPUNGELIB(GM_UNIQUENAME(Expunge), 0)
*/
