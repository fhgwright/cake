/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <proto/exec.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(BOOL, PrintFault,

/*  SYNOPSIS */
	AROS_LHA(LONG,   code,   D1),
	AROS_LHA(STRPTR, header, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 79, Dos)

/*  FUNCTION
	Prints the header and the text associated with the error code to
	the console (buffered), then sets the value returned by IoErr() to
	the error code given.

    INPUTS
	code   - Error code.
	header - Text to print before the error message. This may be NULL
                 in which case only the error message is printed.

    RESULT
	!=0 if all went well. 0 on failure.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	Fault()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    struct Process *me=(struct Process *)FindTask(NULL);
    BPTR stream=me->pr_CES?me->pr_CES:me->pr_COS;
    UBYTE buffer[80];
    BOOL ret;

    /* Fault() will do all the formatting of the string */
    Fault(code, header, buffer, 80);

    if(!FPuts(stream, buffer) && !FPuts(stream, "\n"))
	ret = TRUE;
    else
	ret = FALSE;

    /* All done. */
    me->pr_Result2=code;
    return ret;

    AROS_LIBFUNC_EXIT
} /* PrintFault */
