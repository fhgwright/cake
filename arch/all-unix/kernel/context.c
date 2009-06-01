#include <aros/debug.h>
#include <aros/kernel.h>
#include <aros/libcall.h>
#include <stddef.h>

#include <proto/exec.h>
#include <proto/kernel.h>

#include "kernel_intern.h"

AROS_LH0I(void *, KrnCreateContext,
          struct KernelBase *, KernelBase, 18, Kernel)
{
    AROS_LIBFUNC_INIT

    return AllocMem(KernelIFace.core_get_context_size(), MEMF_PUBLIC | MEMF_CLEAR);
    
    AROS_LIBFUNC_EXIT
}

AROS_LH1I(void, KrnDeleteContext,
          AROS_LHA(void *, vctx, A0),
          struct KernelBase *, KernelBase, 19, Kernel)
{
    AROS_LIBFUNC_INIT

    FreeMem(vctx, KernelIFace.core_get_context_size());

    AROS_LIBFUNC_EXIT
}

AROS_LH1I(void, KrnPrintContext,
          AROS_LHA(void *, vctx, A0),
          struct KernelBase *, KernelBase, 20, Kernel)
{
    AROS_LIBFUNC_INIT

    KernelIFace.core_print_context(vctx);

    AROS_LIBFUNC_EXIT
}

AROS_LH3I(void, KrnPrepareContext,
          AROS_LHA(void *, vctx, A0),
          AROS_LHA(void *, sp,   A1),
          AROS_LHA(void *, pc,   A2),
          struct KernelBase *, KernelBase, 21, Kernel)
{
    AROS_LIBFUNC_INIT

    KernelIFace.core_prepare_context(vctx, sp, pc);

    AROS_LIBFUNC_EXIT
}
