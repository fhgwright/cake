#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH3(void, OutputUnwantedString,
        
        AROS_LHA(struct TestSuite *, ts,       A0),
        AROS_LHA(STRPTR,             got,      A1),
        AROS_LHA(LONG,               glen,     D0),
        
        struct TestBase *, TestBase, 14, Test)
{
    AROS_LIBFUNC_INIT

    if (ts->ts_Output == NULL)
        return;

    FPrintf(ts->ts_Output, "# got unwanted: %.*s\n", glen, got);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
