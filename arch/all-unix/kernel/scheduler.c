#define DEBUG 0

#include <aros/debug.h>
#include <aros/kernel.h>
#include <aros/libcall.h>
#include <exec/execbase.h>
#include <hardware/intbits.h>
#include "kernel_intern.h"
#include "syscall.h"

AROS_LH0(KRN_SchedType, KrnGetScheduler,
         struct KernelBase *, KernelBase, 1, Kernel)
{
    AROS_LIBFUNC_INIT
    
    return SCHED_RR;
    
    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, KrnSetScheduler,
         AROS_LHA(KRN_SchedType, sched, D0),
         struct KernelBase *, KernelBase, 2, Kernel)
{
    AROS_LIBFUNC_INIT

    /* Cannot set scheduler yet */
    
    AROS_LIBFUNC_EXIT
}

AROS_LH0(void, KrnCause,
         struct KernelBase *, KernelBase, 3, Kernel)
{
    AROS_LIBFUNC_INIT
    
    D(bug("[kernel] in KrnCause()\n"));
    KernelIFace.core_syscall(sc_CAUSE);

    AROS_LIBFUNC_EXIT
}

AROS_LH0(void , KrnDispatch,
         struct KernelBase *, KernelBase, 4, Kernel)
{
    AROS_LIBFUNC_INIT

    D(bug("[kernel] in KrnDispatch()\n"));
    KernelIFace.core_syscall(sc_DISPATCH);

    AROS_LIBFUNC_EXIT
}

AROS_LH0(void, KrnSwitch,
         struct KernelBase *, KernelBase, 5, Kernel)
{
    AROS_LIBFUNC_INIT

    D(bug("[kernel] in KrnSwitch()\n"));
    KernelIFace.core_syscall(sc_SWITCH);
    
    AROS_LIBFUNC_EXIT
}

AROS_LH0(void, KrnSchedule,
         struct KernelBase *, KernelBase, 6, Kernel)
{
    AROS_LIBFUNC_INIT

    D(bug("[kernel] in KrnSchedule()\n"));
    KernelIFace.core_syscall(sc_SCHEDULE);
        
    AROS_LIBFUNC_EXIT
}
