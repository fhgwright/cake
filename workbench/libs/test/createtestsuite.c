#include <exec/types.h>
#include <libraries/test.h>
#include <utility/tagitem.h>

#include <proto/exec.h>
#include <proto/utility.h>

#include "test_intern.h"

        AROS_LH1(struct TestSuite *, CreateTestSuite,
        
        AROS_LHA(struct TagItem *, tags,  A0),
        
        struct TestBase *, TestBase, 5, Test)
{
    AROS_LIBFUNC_INIT

    APTR pool;
    struct TestSuite *ts;
    struct TagItem *tag;
    TestFunc **tf;
    struct TestFuncNode *tfn;

    if ((pool = CreatePool(MEMF_PUBLIC | MEMF_CLEAR, 4096, 2048)) == NULL)
        return NULL;

    if ((ts = AllocPooled(pool, sizeof(struct TestSuite))) == NULL) {
        DeletePool(pool);
        return NULL;
    }

    ts->ts_Pool = pool;
    ts->ts_Plan = -1;
    NEWLIST(&ts->ts_Funcs);

    while ((tag = NextTagItem(&tags)) != NULL) {
        switch (tag->ti_Tag) {
            case TS_Plan:
                ts->ts_Plan = (LONG) tag->ti_Data;
                break;

            case TS_Functions:
                for (tf = (TestFunc **) tag->ti_Data; *tf != NULL; tf++) {
                    if ((tfn = AllocPooled(pool, sizeof(struct TestFuncNode))) == NULL) {
                        DeletePool(pool);
                        return NULL;
                    }

                    tfn->tfn_Func = *tf;
                    ADDTAIL(&ts->ts_Funcs, tfn);
                }

                break;

            case TS_Startup:
                ts->ts_Startup = (TestFunc *) tag->ti_Data;
                break;

            case TS_Shutdown:
                ts->ts_Shutdown = (TestFunc *) tag->ti_Data;
                break;

            case TS_Output:
                ts->ts_Output = (BPTR) tag->ti_Data;
                break;
        }
    }

    return ts;

    AROS_LIBFUNC_EXIT
} /* CreateTestSuite */
