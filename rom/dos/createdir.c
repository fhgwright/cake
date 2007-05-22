/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Create a new directory.
    Lang: English
*/

#include <exec/memory.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(BPTR, CreateDir,

/*  SYNOPSIS */
	AROS_LHA(CONST_STRPTR, name, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 20, Dos)

/*  FUNCTION
	Creates a new directory under the given name. If all went an
	exclusive lock on the new diretory is returned.

    INPUTS
	name  -- NUL terminated name.

    RESULT
	Exclusive lock to the new directory or 0 if couldn't be created.
	IoErr() gives additional information in that case.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct FileLock *fl;

    /* Get pointer to I/O request. Use stackspace for now. */
    struct IOFileSys iofs;

    /* Allocate memory for lock */
    if ((fl = (struct FileLock *) AllocMem(sizeof(struct FileLock), MEMF_CLEAR)) == NULL) {
        SetIoErr(ERROR_NO_FREE_STORE);
        return NULL;
    }

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_CREATE_DIR, DOSBase);
    iofs.io_Union.io_CREATE_DIR.io_Protection = 0;

    if(DoName(&iofs, name, DOSBase) != 0) {
        FreeMem(fl, sizeof(struct FileLock));
        return NULL;
    }

    fl->fl_Unit   = iofs.IOFS.io_Unit;
    fl->fl_Device = iofs.IOFS.io_Device;

    return MKBADDR(fl);

    AROS_LIBFUNC_EXIT
} /* CreateDir */

