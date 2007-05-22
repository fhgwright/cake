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

	AROS_LH2(LONG, SameLock,

/*  SYNOPSIS */
	AROS_LHA(BPTR, lock1, D1),
	AROS_LHA(BPTR, lock2, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 70, Dos)

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
    
    struct IOFileSys iofs;
    struct FileHandle *fl1;
    struct FileHandle *fl2;

    if(!SameDevice(lock1, lock2))
    	return LOCK_DIFFERENT;
	
    fl1 = (struct FileLock *)BADDR(lock1);
    fl2 = (struct FileLock *)BADDR(lock2);
	
    /* Check if it is the same lock */

    /* Prepare I/O request. */
    InitIOFS(&iofs, FSA_SAME_LOCK, DOSBase);

    iofs.IOFS.io_Device = fl1->fl_Device;
    iofs.IOFS.io_Unit   = fl1->fl_Unit;
    iofs.io_Union.io_SAME_LOCK.io_Lock[0] = fl1->fl_Unit;
    iofs.io_Union.io_SAME_LOCK.io_Lock[1] = fl2->fl_Unit;
    iofs.io_Union.io_SAME_LOCK.io_Same = LOCK_DIFFERENT;

    /* Send the request. */
    DosDoIO(&iofs.IOFS);

    /* Set error code and return */
    SetIoErr(iofs.io_DosError);

    if(iofs.io_DosError != 0)
	return LOCK_DIFFERENT;
    else
    {
	if(iofs.io_Union.io_SAME_LOCK.io_Same == LOCK_SAME)
	    return LOCK_SAME;
	else
	    return LOCK_SAME_VOLUME;
    }
    
    return 0;

    AROS_LIBFUNC_EXIT
} /* SameLock */
