/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.3  1996/08/13 13:52:52  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced __AROS_LA by __AROS_LHA

    Revision 1.2  1996/08/01 17:40:59  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <clib/exec_protos.h>
#include <dos/dos.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
	#include <clib/dos_protos.h>

	__AROS_LH1(void, UnLoadSeg,

/*  SYNOPSIS */
	__AROS_LHA(BPTR, seglist, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 26, Dos)

/*  FUNCTION
	Free a segment list allocated with LoadSeg().

    INPUTS
	seglist - The segment list.

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	LoadSeg()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    __AROS_FUNC_INIT
    __AROS_BASE_EXT_DECL(struct DosLibrary *,DOSBase)

    BPTR next;

    while(seglist)
    {
	next=*(BPTR *)BADDR(seglist);
	FreeVec(BADDR(seglist));
	seglist=next;
    }

    __AROS_FUNC_EXIT
} /* UnLoadSeg */
