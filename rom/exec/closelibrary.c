/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <aros/config.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <aros/libcall.h>
#include <proto/exec.h>

/*****************************************************************************

    NAME */

	AROS_LH1(void, CloseLibrary,

/*  SYNOPSIS */
	AROS_LHA(struct Library *, library, A1),

/*  LOCATION */
	struct ExecBase *, SysBase, 69, Exec)

/*  FUNCTION
	Closes a previously opened library. It is legal to call this function
	with a NULL pointer.

    INPUTS
	library - Pointer to library structure or NULL.

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	OpenLibrary().

    INTERNALS

    HISTORY

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Something to do? */
    if(library!=NULL)
    {
	/* Single-thread the close routine. */
	Forbid();

	/* Do the close */
	(void)AROS_LVO_CALL0(BPTR,struct Library,library,2,);
	/*
	    Normally you'd expect the library to be expunged if this returns
	    non-zero, but this is only exec which doesn't know anything about
	    seglists - therefore dos.library has to SetFunction() into this
	    vector for the additional functionality.
	*/

	/* All done. */
	Permit();
    }


#if (AROS_FLAVOUR == AROS_FLAVOUR_NATIVE)
    /*
	Kludge to force the library base to register d0. Ramlib patches this
	vector for seglist expunge capability and expects the library base in
	d0 after it has called the original (this) function.
    */
    {
	/* Put the library base in register d0 */
	register struct Library *ret __asm("d0") = library;

	/* Make sure the above assignment isn't optimized away */
	asm volatile("": : "r" (ret));
    }
#endif

    AROS_LIBFUNC_EXIT
} /* CloseLibrary */

