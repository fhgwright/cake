#include "text_intern.h"

#include "libdefs.h"

extern SAVEDS STDARGS struct IClass *ObtainEngine(void);

/***************************************************************************************************/

AROS_LH0(struct IClass *, ObtainEngine,
         LIBBASETYPEPTR, LIBBASE, 5, BASENAME)
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(LIBBASETYPEPTR, LIBBASE)

    return ObtainEngine();
    
    AROS_LIBFUNC_EXIT
}

/***************************************************************************************************/
