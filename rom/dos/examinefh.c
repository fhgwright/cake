/*
    Copyright © 1995-2007, The AROS Development Team. All rights reserved.
    $Id: /aros/lock/src/rom/dos/examine.c 26332 2007-05-07T19:49:07.534739Z verhaegs  $

    Desc: dos.library function Examine().
    Lang: English
*/
#include <exec/memory.h>
#include <proto/exec.h>
#include <dos/filesystem.h>
#include <dos/exall.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BOOL, ExamineFH,

 /* SYNOPSIS */
	AROS_LHA(BPTR                  , file, D1),
	AROS_LHA(struct FileInfoBlock *, fib,  D2),

 /* LOCATION */
	struct DosLibrary *, DOSBase, 65, Dos)

 /* FUNCTION

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

    LONG err;

    BPTR lock = BADDR(DupLockFromFH(file));

    err = Examine(lock, fib);

    UnLock(lock);

    return err;

    AROS_LIBFUNC_EXIT
} /* ExamineFH */
