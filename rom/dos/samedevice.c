/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BOOL, SameDevice,

/*  SYNOPSIS */
	AROS_LHA(BPTR, lock1, D1),
	AROS_LHA(BPTR, lock2, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 164, Dos)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    
    struct FileLock *fl1, *fl2;
    
    if (lock1 == NULL || lock2 == NULL)
    	return DOSFALSE;
	
    fl1 = (struct FileLock *)BADDR(lock1);
    fl2 = (struct FileLock *)BADDR(lock2);

    /* XXX is this check good enough? */
    if (FL_DEVICE(fl1) != FL_DEVICE(fl2))
    	return DOSTRUE;
    

    return DOSFALSE;
    AROS_LIBFUNC_EXIT
} /* SameDevice */
