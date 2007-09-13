#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH3(void, OutputExpectedLong,
        
        AROS_LHA(struct TestSuite *, ts,       A0),
        AROS_LHA(LONG,               got,      D0),
        AROS_LHA(LONG,               expected, D1),
        
        struct TestBase *, TestBase, 11, Test)
{
    AROS_LIBFUNC_INIT

    if (ts->ts_Output == NULL)
        return;

    FPrintf(ts->ts_Output, "#          got: %ld\n", got);
    FPrintf(ts->ts_Output, "#     expected: %ld\n", expected);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
