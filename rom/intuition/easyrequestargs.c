/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: Intuition function EasyRequestArgs()
    Lang: english
*/

/*****************************************************************************

    NAME */
#include <proto/intuition.h>
#include <exec/types.h>
#include <intuition/intuition.h>

	AROS_LH4(LONG, EasyRequestArgs,

/*  SYNOPSIS */
	AROS_LHA(struct Window     *, Window, A0),
	AROS_LHA(struct EasyStruct *, easyStruct, A1),
	AROS_LHA(ULONG             *, IDCMP_ptr, A2),
	AROS_LHA(APTR               , ArgList, A3),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 98, Intuition)

/*  FUNCTION
	Opens and handles a requester, which provides one or more choices.
	It blocks the application until the user closes the requester.
	Returned is a integer indicating which gadget had been selected.

    INPUTS
	Window - A reference window. If NULL, the requester opens on
	         the default public screen.
	easyStruct - The EasyStruct structure (<intuition/intuition.h>),
	             which describes the requester.
	IDCMP_Ptr - Pointer to IDCMP flags, which satisfy the requester,
	            too. This is useful for requesters, which want to
	            listen to disk changes, etc. The contents of this
	            pointer is set to the IDCMP flag, which caused the
	            requester to close.
	ArgList - The arguments for easyStruct->es_TextFormat.

    RESULT
	-1, if one of the IDCMP flags of idcmpPTR was set.
	 0, if the rightmost button was clicked or an error occured.
	 n, if the n-th button from the left was clicked.

    NOTES

    EXAMPLE

    BUGS
	Gadget placing is still untidy.

    SEE ALSO
	BuildEasyRequestArgs()

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

    LONG result;
    struct Window *req;

    req = BuildEasyRequestArgs(Window, easyStruct, *IDCMP_ptr, ArgList);
    if (!req)
        return 0L;

    while ((result = SysReqHandler(req, IDCMP_ptr, TRUE)) == -2);

    FreeSysRequest(req);

    return result;
    AROS_LIBFUNC_EXIT
} /* EasyRequestArgs */
