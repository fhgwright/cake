/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Open a file from a lock
    Lang: english
*/
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/stdio.h>

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(BPTR, OpenFromLock,

/*  SYNOPSIS */
	AROS_LHA(BPTR, lock, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 63, Dos)

/*  FUNCTION
	Convert a lock into a filehandle. If all went well the lock
	will be gone. In case of an error it must still be freed.

    INPUTS
	lock - Lock to convert.

    RESULT
	New filehandle or 0 in case of an error. IoErr() will give
	additional information in that case.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct FileHandle *fh;

    if ((fh = AllocDosObject(DOS_FILEHANDLE, NULL)) == NULL) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return NULL;
    }

    fh->fh_Arg1 = lock;

    if (IsInteractive(MKBADDR(fh)))
        SetVBuf(MKBADDR(fh), NULL, BUF_LINE, -1);

    return MKBADDR(fh);

    AROS_LIBFUNC_EXIT
} /* OpenFromLock */
