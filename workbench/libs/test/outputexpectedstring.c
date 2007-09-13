#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH5(void, OutputExpectedString,
        
        AROS_LHA(struct TestSuite *, ts,       A0),
        AROS_LHA(STRPTR,             got,      A1),
        AROS_LHA(LONG,               glen,     D0),
        AROS_LHA(STRPTR,             expected, A2),
        AROS_LHA(LONG,               elen,     D1),
        
        struct TestBase *, TestBase, 12, Test)
{
    AROS_LIBFUNC_INIT

    if (ts->ts_Output == NULL)
        return;

    FPrintf(ts->ts_Output, "#          got: %.*s\n", glen, got);
    FPrintf(ts->ts_Output, "#     expected: %.*s\n", elen, expected);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
