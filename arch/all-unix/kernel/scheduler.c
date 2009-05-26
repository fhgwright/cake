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

    in_supervisor++;
    core_Cause(SysBase);
    in_supervisor--;

    AROS_LIBFUNC_EXIT
}

AROS_LH0(void , KrnDispatch,
         struct KernelBase *, KernelBase, 4, Kernel)
{
    AROS_LIBFUNC_INIT

    ucontext_t ctx;

    D(bug("[KRN] KrnDispatch()\n"));

    getcontext(&ctx);

    in_supervisor++;
    core_Dispatch(&ctx);
    in_supervisor--;

    AROS_LIBFUNC_EXIT
}

AROS_LH0(void, KrnSwitch,
         struct KernelBase *, KernelBase, 5, Kernel)
{
    AROS_LIBFUNC_INIT

    ucontext_t ctx;

    D(bug("[KRN] KrnSwitch()\n"));

    getcontext(&ctx);

    in_supervisor++;
    core_Switch(&ctx);
    in_supervisor--;
    
    AROS_LIBFUNC_EXIT
}

AROS_LH0(void, KrnSchedule,
         struct KernelBase *, KernelBase, 6, Kernel)
{
    AROS_LIBFUNC_INIT

    ucontext_t ctx;

    D(bug("[KRN] KrnSchedule()\n"));

    getcontext(&ctx);

    in_supervisor++;
    core_Schedule(&ctx);
    in_supervisor--;
    
    AROS_LIBFUNC_EXIT
}
