#ifndef THREAD_INTERN_H
#define THREAD_INTERN_H 1

#define DEBUG 1
#include <aros/debug.h>

#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <stdint.h>

#include <libraries/thread.h>

struct _ThreadCondition {
};

typedef struct SignalSemaphore  *_Mutex;
typedef struct _ThreadCondition *_ThreadCondition;

struct ThreadBase {
    struct Library library;
};

#endif
