/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id: /aros/lock/src/rom/dos/namefromlock.c 26332 2007-05-07T19:49:07.534739Z verhaegs  $

    Desc: Retrieve thew full pathname from a lock.
    Lang: english
*/
#include <proto/exec.h>
#include <dos/exall.h>
#include <dos/filesystem.h>
#include "dos_intern.h"
#include <aros/debug.h>

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH3(LONG, NameFromFH,

 /* SYNOPSIS */
	AROS_LHA(BPTR  , file, D1),
	AROS_LHA(STRPTR, buffer, D2),
	AROS_LHA(LONG  , len, D3),

 /* LOCATION */
	struct DosLibrary *, DOSBase, 68, Dos)

 /* FUNCTION
	Get the full path name associated with file-handle into a
	user supplied buffer.

    INPUTS
	file   - File-handle to file or directory.
	buffer - Buffer to fill. Contains a NUL terminated string if
		 all went well.
	length - Size of the buffer in bytes.

    RESULT
	!=0 if all went well, 0 in case of an error. IoErr() will
	give additional information in that case.

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    struct FileHandle *fh = (struct FileHandle *) BADDR(file);
    struct FileLock *fl;
    LONG err;

    fl = AllocMem(sizeof(struct FileLock), MEMF_CLEAR);
    FL_DEVICE(fl) = fh->fh_Device;
    FL_UNIT(fl) = fh->fh_Unit;

    err = NameFromLock(MKBADDR(fl), buffer, len);

    FreeMem(fl, sizeof(struct FileLock));

    return err;

    AROS_LIBFUNC_EXIT
    
} /* NameFromFH */
