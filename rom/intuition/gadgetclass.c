/* AROS gadgetclass implementation
 * 10/25/96 caldi@usa.nai.net
 */

#include <exec/types.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/cghooks.h>
#include <intuition/icclass.h>

#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>

#include <utility/tagitem.h>
#include <utility/hooks.h>

#include <clib/macros.h>

#ifdef _SASC
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#elif __GNUC__
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#endif

#ifdef _AROS
#include <aros/asmcall.h>
#include <clib/alib_protos.h>
#include "intuition_intern.h"
#include "gadgets.h"
#endif

/****************************************************************************/

/* Some handy transparent base class object casting defines.
 */
#define G(o)  ((struct Gadget *)o)
#define EG(o) ((struct ExtGadget *)o)
#define IM(o) ((struct Image *)o)

/****************************************************************************/

#undef IntuitionBase
#define IntuitionBase	 ((struct IntuitionBase *)(cl->cl_UserData))

/* set gadget attributes
 */
static ULONG set_gadgetclass(Class *cl, Object *o, struct opSet *msg)
{
    struct TagItem *tstate = msg->ops_AttrList;
    struct TagItem *tag;
    IPTR  tidata;
    ULONG retval = 0UL; /* set to non-zero to signal visual changes */

    while ( (tag = NextTagItem(&tstate)) )
    {
	tidata = tag->ti_Data;

	switch(tag->ti_Tag)
	{
	case GA_Left:
	    EG(o)->LeftEdge = (WORD)tidata;
	    retval = 1UL;
	    break;

	case GA_Top:
	    EG(o)->TopEdge = (WORD)tidata;
	    retval = 1UL;
	    break;

	case GA_Width:
	    EG(o)->Width = (WORD)tidata;
	    retval = 1UL;
	    break;

	case GA_Height:
	    EG(o)->Height = (WORD)tidata;
	    retval = 1UL;
	    break;

	case GA_RelRight:
	    EG(o)->LeftEdge = (WORD)tidata;
	    EG(o)->Flags |= GFLG_RELRIGHT;
	    retval = 1UL;
	    break;

	case GA_RelBottom:
	    EG(o)->TopEdge = (WORD)tidata;
	    EG(o)->Flags |= GFLG_RELBOTTOM;
	    retval = 1UL;
	    break;

	case GA_RelWidth:
	    EG(o)->Width = (WORD)tidata;
	    EG(o)->Flags |= GFLG_RELWIDTH;
	    retval = 1UL;
	    break;

	case GA_RelHeight:
	    EG(o)->Height = (WORD)tidata;
	    EG(o)->Flags |= GFLG_RELHEIGHT;
	    retval = 1UL;
	    break;

	case GA_Previous:
	    if( (tidata != 0L) && (msg->MethodID == OM_NEW) )
	    {
		((struct ExtGadget *)tidata)->NextGadget = EG(o);
	    }
	    break;

	case GA_IntuiText:
	    EG(o)->GadgetText = (struct IntuiText *)tidata;
	    EG(o)->Flags &= ~GFLG_LABELMASK;
	    EG(o)->Flags |= GFLG_LABELITEXT;
	    retval = 1UL;
	    break;

	case GA_Text:
	    EG(o)->GadgetText = (struct IntuiText *)tidata;
	    EG(o)->Flags &= ~GFLG_LABELMASK;
	    EG(o)->Flags |= GFLG_LABELSTRING;
	    retval = 1UL;
	    break;

	case GA_LabelImage:
	    EG(o)->GadgetText = (struct IntuiText *)tidata;
	    EG(o)->Flags &= ~GFLG_LABELMASK;
	    EG(o)->Flags |= GFLG_LABELIMAGE;
	    retval = 1UL;
	    break;

	case GA_Image:
	    EG(o)->GadgetRender = (APTR)tidata;
	    EG(o)->Flags |= GFLG_GADGIMAGE;
	    retval = 1UL;
	    break;

	case GA_Border:
	    EG(o)->GadgetRender = (APTR)tidata;
	    EG(o)->Flags &= ~GFLG_GADGIMAGE;
	    retval = 1UL;
	    break;

	case GA_SelectRender:
	    EG(o)->SelectRender = (APTR)tidata;
	    EG(o)->Flags |= (GFLG_GADGIMAGE & GFLG_GADGHIMAGE);
	    retval = 1UL;
	    break;

	case GA_SpecialInfo:
	    EG(o)->SpecialInfo = (APTR)tidata;
	    break;

	case GA_GZZGadget:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->GadgetType |= GTYP_GZZGADGET;
	    else
		EG(o)->GadgetType &= ~GTYP_GZZGADGET;
	    break;

	case GA_SysGadget:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->GadgetType |= GTYP_SYSGADGET;
	    else
		EG(o)->GadgetType &= ~GTYP_SYSGADGET;
	    break;

	case GA_Disabled:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Flags |= GFLG_SELECTED;
	    else
		EG(o)->Flags &= ~GFLG_SELECTED;
	    retval = 1UL;
	    break;

	case GA_Selected:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Flags |= GFLG_DISABLED;
	    else
		EG(o)->Flags &= ~GFLG_DISABLED;
	    retval = 1UL;
	    break;

	case GA_EndGadget:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_ENDGADGET;
	    else
		EG(o)->Activation &= ~GACT_ENDGADGET;
	    break;

	case GA_Immediate:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_IMMEDIATE;
	    else
		EG(o)->Activation &= ~GACT_IMMEDIATE;
	    break;

	case GA_RelVerify:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_RELVERIFY;
	    else
		EG(o)->Activation &= ~GACT_RELVERIFY;
	    break;

	case GA_FollowMouse:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_FOLLOWMOUSE;
	    else
		EG(o)->Activation &= ~GACT_FOLLOWMOUSE;
	    break;

	case GA_RightBorder:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_RIGHTBORDER;
	    else
		EG(o)->Activation &= ~GACT_RIGHTBORDER;
	    break;

	case GA_LeftBorder:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_LEFTBORDER;
	    else
		EG(o)->Activation &= ~GACT_LEFTBORDER;
	    break;

	case GA_TopBorder:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_TOPBORDER;
	    else
		EG(o)->Activation &= ~GACT_TOPBORDER;
	    break;

	case GA_BottomBorder:
	    if ( (BOOL)tidata != FALSE )
		EG(o)->Activation |= GACT_BOTTOMBORDER;
	    else
		EG(o)->Activation &= ~GACT_BOTTOMBORDER;
	    break;

	case GA_Highlight:
	    EG(o)->Flags &= ~GFLG_GADGHIGHBITS;
	    EG(o)->Flags |= tidata;
	    break;

	case GA_SysGType:
	    EG(o)->GadgetType &= ~GTYP_SYSTYPEMASK;
	    EG(o)->GadgetType |= tidata;
	    break;

	case GA_ID:
	    EG(o)->GadgetID = tidata;
	    break;

	case GA_UserData:
	    EG(o)->UserData = (APTR)tidata;
	    break;
	}
    }

    return retval;
}


/* get gadget attributes - gadgetclass really has no gettable
 * attributes, but we will implement some useful ones anyway. ;0
 */
static ULONG get_gadgetclass(Class *cl, Object *o, struct opGet *msg)
{
    ULONG retval = 1UL;

    switch (msg->opg_AttrID)
    {
    case GA_Left:
    case GA_RelRight:
	*msg->opg_Storage = (ULONG) EG(o)->LeftEdge;
	break;

    case GA_Top:
    case GA_RelBottom:
	*msg->opg_Storage = (ULONG) EG(o)->TopEdge;
	break;

    case GA_Width:
    case GA_RelWidth:
	*msg->opg_Storage = (ULONG) EG(o)->Width;
	break;

    case GA_Height:
    case GA_RelHeight:
	*msg->opg_Storage = (ULONG) EG(o)->Height;
	break;

    case GA_Selected:
	*msg->opg_Storage = (ULONG)((BOOL)(EG(o)->Flags & GFLG_SELECTED));
	break;

    case GA_Disabled:
	*msg->opg_Storage = (ULONG)((BOOL)(EG(o)->Flags & GFLG_DISABLED));
	break;

    default:
	*msg->opg_Storage = (ULONG)NULL;
	retval = 0UL;
	break;
    }

    return(retval);
}

/* test if we should try to activate this gadget...
 */
static ULONG hittest_gadgetclass(Class *cl, Object *o, struct gpHitTest *gpht)
{
    struct IBox container;

    GetGadgetIBox(o, gpht->gpht_GInfo, &container);

    if ( (gpht->gpht_Mouse.X >= container.Left) &&
	 (gpht->gpht_Mouse.X <  container.Left + container.Width) &&
	 (gpht->gpht_Mouse.Y >= container.Top) &&
	 (gpht->gpht_Mouse.Y <  container.Top + container.Height)
    )
    {
	return(GMR_GADGETHIT);
    }

    return(0UL);
}


/* gadgetclass boopsi dispatcher
 */
AROS_UFH3(static IPTR, dispatch_gadgetclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    IPTR retval = 0UL;

    switch(msg->MethodID)
    {
    case GM_RENDER:
	retval = 1UL;
	break;

    case GM_LAYOUT:
    case GM_DOMAIN:
    case GM_GOACTIVE:
    case GM_HANDLEINPUT:
    case GM_GOINACTIVE:
	/* our subclasses handle these methods */
	break;

    case GM_HITTEST:
	if( (G(o)->Flags & GFLG_DISABLED) == 0 )
	{
	    retval = (IPTR)hittest_gadgetclass(cl, o, (struct gpHitTest *)msg);
	}
	break;

    case OM_NEW:
	retval = DoSuperMethodA(cl, o, msg);

	if (retval)
	{
	    /* set some defaults */
	    EG(retval)->NextGadget = NULL;
	    EG(retval)->LeftEdge   = 0;
	    EG(retval)->TopEdge    = 0;
	    EG(retval)->Width      = 6;
	    EG(retval)->Height     = 4;
	    EG(retval)->Flags      = GFLG_EXTENDED;
	    EG(retval)->MoreFlags  = 0UL;
	    EG(retval)->GadgetType = GTYP_CUSTOMGADGET;

	    /* Handle our special tags - overrides defaults */
	    set_gadgetclass(cl, (Object*)retval, (struct opSet *)msg);
	}
	break;

    case OM_SET:
    case OM_UPDATE:
	retval = DoSuperMethodA(cl, o, msg);
	retval += (IPTR)set_gadgetclass(cl, o, (struct opSet *)msg);
	break;

    case OM_GET:
	retval = (IPTR)get_gadgetclass(cl, o, (struct opGet *)msg);
	break;

    default:
	retval = DoSuperMethodA(cl, o, msg);
	break;
    } /* switch */

    return retval;
}  /* dispatch_gadgetclass */


#undef IntuitionBase

/****************************************************************************/

/* Initialize our image class. */
struct IClass *InitGadgetClass (struct IntuitionBase * IntuitionBase)
{
    struct IClass *cl = NULL;

    /* This is the code to make the gadgetclass...
     */
    if ((cl = MakeClass(GADGETCLASS, ROOTCLASS, NULL, sizeof(struct ExtGadget), 0)))
    {
	cl->cl_Dispatcher.h_Entry    = (APTR)AROS_ASMSYMNAME(dispatch_gadgetclass);
	cl->cl_Dispatcher.h_SubEntry = NULL;
	cl->cl_UserData 	     = (IPTR)IntuitionBase;

	AddClass (cl);
    }

    return (cl);
}

