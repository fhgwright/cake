#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH2(void, OutputUnwantedLong,
        
        AROS_LHA(struct TestSuite *, ts,       A0),
        AROS_LHA(LONG,               got,      D0),
        
        struct TestBase *, TestBase, 13, Test)
{
    AROS_LIBFUNC_INIT

    if (ts->ts_Output == NULL)
        return;

    FPrintf(ts->ts_Output, "# got unwanted: %ld\n", got);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
