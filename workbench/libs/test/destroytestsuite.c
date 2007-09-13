#include <libraries/test.h>
#include <proto/exec.h>

#include "test_intern.h"

        AROS_LH1(void, DestroyTestSuite,
        
        AROS_LHA(struct TestSuite *, ts, A0),
        
        struct TestBase *, TestBase, 7, Test)
{
    AROS_LIBFUNC_INIT

    DeletePool(ts->ts_Pool);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
