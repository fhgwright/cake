/*
    (C) 1995-96 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
	#include <clib/intuition_protos.h>

	AROS_LH5(void, ChangeWindowBox,

/*  SYNOPSIS */
	AROS_LHA(struct Window *, window, A0),
	AROS_LHA(LONG           , left, D0),
	AROS_LHA(LONG           , top, D1),
	AROS_LHA(LONG           , width, D2),
	AROS_LHA(LONG           , height, D3),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 81, Intuition)

/*  FUNCTION
	Set the new position and size of a window in one call.

    INPUTS
	window - Change this window
	left, top - New position
	width, height - New size

    RESULT

    NOTES
	The window will not change its position immediately but only when the
	next input event is received.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    intui_ChangeWindowBox (window, left, top, width, height);

    AROS_LIBFUNC_EXIT
} /* ChangeWindowBox */
