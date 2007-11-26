#ifndef LIBRARIES_TEST_H
#define LIBRARIES_TEST_H

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <dos/bptr.h>
#include <utility/tagitem.h>

typedef BOOL (*TestFunc)(struct TestSuite *ts, void **arg);

#define TESTNAME(name) test_##name
#define TESTFUNC(name) BOOL test_##name (struct TestSuite *ts, void **arg)

struct TestSuite {
    APTR            ts_Pool;

    LONG            ts_Plan;

    struct MinList  ts_Funcs;

    TestFunc        *ts_Startup;
    TestFunc        *ts_Shutdown;

    BPTR            ts_Output;

    LONG            ts_Count;
    LONG            ts_Failed;
};

struct TestFuncNode {
    struct MinNode  tfn_Node;

    TestFunc        *tfn_Func;
};

#define TS_Plan         (TAG_USER)
#define TS_Functions    (TAG_USER + 1)
#define TS_Startup      (TAG_USER + 2)
#define TS_Shutdown     (TAG_USER + 3)
#define TS_Output       (TAG_USER + 4)

#endif
