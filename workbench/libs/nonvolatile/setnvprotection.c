/*
    Copyright (C) 2000-2001 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/

#include <dos/dosextens.h>

/*****************************************************************************

    NAME */

#include <libraries/nonvolatile.h>
#include <proto/exec.h>
#include <proto/nvdisk.h>
#include "nonvolatile_intern.h"

#include <string.h>

AROS_LH4(BOOL, SetNVProtection,

/*  SYNOPSIS */

	AROS_LHA(STRPTR, appName,        A0),
        AROS_LHA(STRPTR, itemName,       A1),
	AROS_LHA(LONG,   mask,           D2),
	AROS_LHA(BOOL,   killRequesters, D1),

/*  LOCATION */

	struct Library *, nvBase, 11, Nonvolatile)

/*  FUNCTION

    Set the protection attributes for a nonvolatile item.

    INPUTS

    appName         --  the application owning the item stored in nonvolatile
                        memory
    itemName        --  the name of the item to change the protection of
    mask            --  the new protection status
    
    killRequesters  --  if TRUE no system requesters will be displayed during
                        the operation of this function

    RESULT

    Success / failure indicator.
    
    NOTES

    The only bit that should currently be used in the 'mask' is the DELETE bit.

    EXAMPLE

    BUGS

    SEE ALSO

    GetNVList(), <libraries/nonvolatile.h>

    INTERNALS

    HISTORY

    April 2000,  SDuvan  --  implemented

******************************************************************************/

{
    AROS_LIBFUNC_INIT

    struct Process *me = (struct Process *)FindTask(NULL);
    APTR oldReq = me->pr_WindowPtr;
    BOOL result;

    if(appName == NULL || itemName == NULL)
	return NULL;

    if(strpbrk(appName, ":/") != NULL ||
       strpbrk(itemName, ":/") != NULL)
	return NULL;

    if(killRequesters)
	me->pr_WindowPtr = (APTR)-1;

    result = SetProtection(appName, itemName, mask);

    if(killRequesters)
	me->pr_WindowPtr = oldReq;

    return result;

    AROS_LIBFUNC_EXIT
} /* SetNVProtection */

