/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.7  1997/05/01 14:14:30  aros
    Added workaround for GCC braindamage. Needed at least for Linux/m68k.

    Revision 1.6  1997/01/27 00:36:19  ldp
    Polish

    Revision 1.5  1996/12/09 13:53:27  aros
    Added empty templates for all missing functions

    Moved #include's into first column

    Revision 1.4  1996/10/24 15:50:27  aros
    Use the official AROS macros over the __AROS versions.

    Revision 1.3  1996/08/13 13:52:46  digulla
    Replaced <dos/dosextens.h> by "dos_intern.h" or added "dos_intern.h"
    Replaced AROS_LA by AROS_LHA

    Revision 1.2  1996/08/01 17:40:50  digulla
    Added standard header for all files

    Desc:
    Lang: english
*/
#include <proto/utility.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(LONG, FindArg,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, template, D1),
	AROS_LHA(STRPTR, keyword,  D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 134, Dos)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    LONG count=0;
    STRPTR key;

    /* Loop over template */
    for(;;)
    {
	/* Compare key to template */
	key=keyword;
	for(;;)
	{
	    UBYTE lkey;

	    /* If the keyword has ended check the template */
	    if(!*key)
	    {
		if(!*template||*template=='='||*template=='/'||*template==',')
		    /* The template has ended, too. Return count. */
		    return count;
		/* The template isn't finished. Stop comparison. */
		break;
	    }
	    /* If the two differ stop comparison. */
	    lkey=ToLower(*key);
	    if(lkey!=ToLower(*template))
		break;
	    /* Go to next character */
	    key++;
	    template++;
	}
	/* Find next keyword in template */
	for(;;)
	{
	    if(!*template)
		return -1;
	    if(*template=='=')
	    {
		/* Alias found */
		template++;
		break;
	    }
	    if(*template==',')
	    {
		/* Next item found */
		template++;
		count++;
		break;
	    }
	    template++;
	}
    }
    AROS_LIBFUNC_EXIT
} /* FindArg */
