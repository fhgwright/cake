#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/test.h>

#include "test_intern.h"

        AROS_LH3(void, RecordTestFailureFileLine,
        
        AROS_LHA(struct TestSuite *, ts,   A0),
        AROS_LHA(const char *,       file, A1),
        AROS_LHA(int,                line, D0),
        
        struct TestBase *, TestBase, 10, Test)
{
    AROS_LIBFUNC_INIT

    RecordTestFailure(ts);

    if (ts->ts_Output != NULL)
        FPrintf(ts->ts_Output, "#     Failed test (%s at line %d)\n", file, line);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
