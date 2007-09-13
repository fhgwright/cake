#include <exec/types.h>
#include <libraries/test.h>
#include <proto/test.h>
#include <proto/dos.h>
#include <stdio.h>

extern TESTFUNC(startup);
extern TESTFUNC(shutdown);

extern TESTFUNC(test1);
extern TESTFUNC(test2);
extern TESTFUNC(test3);

TESTFUNC(startup) {
    return TRUE;
}

TESTFUNC(shutdown) {
    return TRUE;
}

TESTFUNC(test1) {
    RecordTestSuccess(ts);
    OutputExpectedLong(ts, 5, 7);
    return TRUE;
}

TESTFUNC(test2) {
    RecordTestFailure(ts);
    OutputExpectedString(ts, "foo", 3, "bar", 3);
    return TRUE;
}

TESTFUNC(test3) {
    RecordTestFailureFileLine(ts, __FILE__, __LINE__);
    OutputUnwantedLong(ts, 12);
    OutputUnwantedString(ts, "baz", 3);
    return TRUE;
}

static TestFunc *tests[] = {
    TESTNAME(test1),
    TESTNAME(test2),
    TESTNAME(test3),
    NULL
};

int main(int argc, char **argv) {
    struct TestSuite *ts;
    struct TagItem tags[] = {
        { TS_Plan,      (IPTR) 3                  },
        { TS_Functions, (IPTR) tests              },
        { TS_Startup,   (IPTR) TESTNAME(startup)  },
        { TS_Shutdown,  (IPTR) TESTNAME(shutdown) },
        { TS_Output,    (IPTR) Output()           },
        { TAG_DONE,     0                         }
    };


    if ((ts = CreateTestSuite(tags)) == NULL) {
        printf("couldn't create test suite\n");
        return 1;
    }

    RunTestSuite(ts);
    DestroyTestSuite(ts);

    return 0;
}
