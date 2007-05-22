/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id: /aros/lock/src/rom/dos/duplock.c 26332 2007-05-07T19:49:07.534739Z verhaegs  $

    Desc: dos.library function DupLock()
    Lang: english
*/
#include <proto/exec.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(BPTR, DupLockFromFH,

 /* SYNOPSIS */
	AROS_LHA(BPTR, fh, D1),

 /* LOCATION */
	struct DosLibrary *, DOSBase, 62, Dos)

 /* FUNCTION
	Try to get a lock on the object selected by the filehandle.

    INPUTS
	fh - filehandle.

    RESULT
	The new lock or 0 in case of an error. IoErr() will give additional
	information in that case.

    NOTES
	This function is identical to DupLock().

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    return DupLock(((struct FileHandle *) BADDR(fh))->fh_Arg1);

    AROS_LIBFUNC_EXIT
} /* DupLockFromFH */
