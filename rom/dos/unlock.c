/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id: /aros/lock/src/rom/dos/close.c 26332 2007-05-07T19:49:07.534739Z verhaegs  $

    Desc:
    Lang: English
*/
#include <proto/exec.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(void, UnLock,

/*  SYNOPSIS */
	AROS_LHA(BPTR, lock, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 15, Dos)

/*  FUNCTION
	Free a lock created with Lock().

    INPUTS
	lock  --  The lock to free

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Get pointer to lock */
    struct FileLock *fl = (struct FileLock *) BADDR(lock);

    /* Get space for I/O request. Use stack for now. */
    struct IOFileSys iofs;

    /* 0 handles are OK */
    if(lock == NULL)
	return;

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_CLOSE, DOSBase);

    iofs.IOFS.io_Device = fl->fl_Device;
    iofs.IOFS.io_Unit	= fl->fl_Unit;

    /* Send the request. No errors possible. */
    DosDoIO(&iofs.IOFS);

    /* Free the lock which was allocated in Lock(), CreateDir() and such. */
    FreeMem(fl, sizeof(struct FileLock));

    AROS_LIBFUNC_EXIT
} /* Close */
