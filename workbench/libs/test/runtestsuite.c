#include <libraries/test.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH1(BOOL, RunTestSuite,
        
        AROS_LHA(struct TestSuite *, ts, A0),
        
        struct TestBase *, TestBase, 6, Test)
{
    AROS_LIBFUNC_INIT

    void *arg;
    struct TestFuncNode *tfn;

    if (ts->ts_Startup != NULL)
        if (AROS_UFC2(BOOL, ts->ts_Startup,
                      AROS_UFCA(struct TestSuite *, ts,   A0),
                      AROS_UFCA(void **,            &arg, A1)) == FALSE)
            return FALSE;

    if (ts->ts_Plan >= 1 && ts->ts_Output != NULL)
        FPrintf(ts->ts_Output, "1..%d\n", ts->ts_Plan);

    ForeachNode(&ts->ts_Funcs, tfn) {
        AROS_UFC2(void, tfn->tfn_Func,
                  AROS_UFCA(struct TestSuite *, ts,   A0),
                  AROS_UFCA(void **,            &arg, A1));
    }

    if (ts->ts_Shutdown != NULL)
        AROS_UFC2(void, ts->ts_Shutdown,
                  AROS_UFCA(struct TestSuite *, ts,   A0),
                  AROS_UFCA(void **,            &arg, A1));

    return FALSE;

    AROS_LIBFUNC_EXIT
} /* RunTestSuite */
