/*
    (C) 1995-97 AROS - The Amiga Research OS
    $Id$

    Desc: AROS colorwheel gadget.
    Lang: english
*/

#define USE_BOOPSI_STUBS
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/colorwheel.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <gadgets/colorwheel.h>
#include <gadgets/gradientslider.h>
#include <utility/tagitem.h>
#include <gadgets/colorwheel.h>
#include <aros/asmcall.h>
#include <stdlib.h> /* abs() */
#include "colorwheel_intern.h"

#undef SDEBUG
#define SDEBUG 0
#undef DEBUG
#define DEBUG 1
#include <aros/debug.h>

#define ColorWheelBase ((struct ColorWheelBase_intern *)(cl->cl_UserData))

#include <clib/boopsistubs.h>


/***************************************************************************************************/

STATIC VOID notify_all(Class *cl, Object *o, struct GadgetInfo *gi, BOOL interim, BOOL userinput)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);
    struct opUpdate		opu;
    struct TagItem		tags[] =
    {
        {GA_ID 			, EG(o)->GadgetID		}, 
        {GA_UserInput		, userinput			},         	
        {WHEEL_Hue		, data->hsb.cw_Hue		},
	{WHEEL_Saturation	, data->hsb.cw_Saturation	},
#if 0
	{WHEEL_Brightness	, data->hsb.cw_Brightness	},
	{WHEEL_Red		, data->rgb.cw_Red		},
	{WHEEL_Green		, data->rgb.cw_Green		},
	{WHEEL_Blue		, data->rgb.cw_Blue		},
#endif
	{TAG_DONE						}
    };
    
    opu.MethodID     = OM_NOTIFY;
    opu.opu_AttrList = tags;
    opu.opu_GInfo    = gi; 
    opu.opu_Flags    = interim ? OPUF_INTERIM : 0;
    
    DoMethodA(o, (Msg) &opu);
}

/***************************************************************************************************/

STATIC IPTR colorwheel_set(Class *cl, Object *o, struct opSet *msg)
{
    struct TagItem 		*tag, *tstate;
    struct ColorWheelData 	*data 	       = INST_DATA(cl, o);
    ULONG			old_brightness;
    BOOL			do_notify      = FALSE;
    BOOL			disabled;
    
    IPTR 			retval         = 0UL;

    EnterFunc(bug("ColorWheel::Set()\n"));

    disabled = (EG(o)->Flags & GFLG_DISABLED) != 0L;
    
    if (msg->MethodID != OM_NEW) retval = DoSuperMethodA(cl, o, msg);
    
    if (data->gradobj)
    {
        IPTR gradval;
	
	/* GRAD_MaxVai's applicability is I (OM_NEW) only, so
	   colorwheel.gadget assumes a maxval of 0xFFFF (default) */
	   
	GetAttr(GRAD_CurVal, data->gradobj, &gradval);
	gradval = 0xFFFF - gradval;
	
	old_brightness = ((ULONG)gradval) * 0x10000 + (ULONG)gradval;
    }
    
    tstate = msg->ops_AttrList; 
    while((tag = NextTagItem((const struct TagItem **)&tstate)))
    {
    	IPTR tidata = tag->ti_Data;
    	   	
    	switch (tag->ti_Tag)
    	{
	    case GA_Disabled:
	    	if(disabled != tidata)
	    	{	    		
	            retval += 1UL;
		}	
		break;

	    case WHEEL_Hue:
	    	data->hsb.cw_Hue = (ULONG)tidata;
		ConvertHSBToRGB(&data->hsb, &data->rgb);
		do_notify = TRUE;
		retval += 1UL;
		break;

	    case WHEEL_Saturation:
	        data->hsb.cw_Saturation = (ULONG)tidata;
		ConvertHSBToRGB(&data->hsb, &data->rgb);
		do_notify = TRUE;
		retval += 1UL;
		break;

	    case WHEEL_Brightness:
	        data->hsb.cw_Brightness = (ULONG)tidata;
		ConvertHSBToRGB(&data->hsb, &data->rgb);
		/* do_notify = TRUE; */
		retval += 1UL;
		break;
		
	    case WHEEL_HSB:
	    	data->hsb = *(struct ColorWheelHSB *)tidata;
		ConvertHSBToRGB(&data->hsb, &data->rgb);
		do_notify = TRUE;
		retval += 1UL;
		break;
		
	    case WHEEL_Red:
	        data->rgb.cw_Red = (ULONG)tidata;
		ConvertRGBToHSB(&data->rgb, &data->hsb);
		do_notify = TRUE;
		retval += 1UL;
		break;
		
	    case WHEEL_Green:
	        data->rgb.cw_Green = (ULONG)tidata;
		ConvertRGBToHSB(&data->rgb, &data->hsb);
		do_notify = TRUE;
		retval += 1UL;
		break;
		
	    case WHEEL_Blue:
	    	data->rgb.cw_Blue = (ULONG)tidata;
		ConvertRGBToHSB(&data->rgb, &data->hsb);
		do_notify = TRUE;
		retval += 1UL;
		break;
	
	    case WHEEL_RGB:
	        data->rgb = *(struct ColorWheelRGB *)tidata;
		ConvertRGBToHSB(&data->rgb, &data->hsb);
		do_notify = TRUE;
		retval += 1UL;
		break;
		
	    case WHEEL_GradientSlider:
	    	data->gradobj = (Object *)tidata;
		break;
		
	    default:
	        break;
		
    	} /* switch (tag->ti_Tag) */
    	
    } /* for (each attr in attrlist) */
    
    if (do_notify) notify_all(cl, o, msg->ops_GInfo, FALSE, FALSE);

    if (data->gradobj && ((msg->MethodID == OM_NEW) || (data->hsb.cw_Brightness != old_brightness)))
    {
        struct TagItem set_tags[] =
	{
	    {GRAD_CurVal , 0	},
	    {TAG_DONE		}
	};

	set_tags[0].ti_Data = 0xFFFF - (data->hsb.cw_Brightness / 0x10000);
	
        DoMethod(data->gradobj, OM_SET, (IPTR)set_tags, (IPTR)msg->ops_GInfo);
    }
    
    if (retval)
    {
        struct RastPort *rp;
	
	if ((rp = ObtainGIRPort(msg->ops_GInfo)))
	{
	    DoMethod(o, GM_RENDER, (IPTR)msg->ops_GInfo, (IPTR)rp, GREDRAW_UPDATE);
	    ReleaseGIRPort(rp);
	}
	
	retval = 0L;
    }    
    ReturnPtr ("ColorWheel::Set", IPTR, retval);
}

/***************************************************************************************************/

STATIC Object *colorwheel_new(Class *cl, Object *o, struct opSet *msg)
{
    EnterFunc(bug("ColorWheel::New()\n"));
    
    o = (Object *)DoSuperMethodA(cl, o, (Msg)msg);
    if (o)
    {
    	struct ColorWheelData *data = INST_DATA(cl, o);
    	
	data->scr = (struct Screen *)GetTagData(WHEEL_Screen, 0, msg->ops_AttrList);
	
	if (GetTagData(WHEEL_BevelBox, FALSE, msg->ops_AttrList))
	{
	    struct TagItem fitags[]=
	    {
               {IA_EdgesOnly	, FALSE		},
	       {IA_FrameType	, FRAME_BUTTON	},
               {TAG_DONE			}
	    };

	    data->frame = NewObjectA(NULL, FRAMEICLASS, fitags);
	}
	
	if (data->scr)
	{
	    data->hsb.cw_Hue        = 0;
	    data->hsb.cw_Saturation = 0xFFFFFFFF;
	    data->hsb.cw_Brightness = 0xFFFFFFFF;
	    
	    data->rgb.cw_Red	    = 0xFFFFFFFF;
	    data->rgb.cw_Green      = 0;
	    data->rgb.cw_Blue       = 0;
	    
	    data->abbrv    =  (STRPTR) GetTagData(WHEEL_Abbrv         , (IPTR)"GCBMRY", msg->ops_AttrList);
	    data->donation = (UWORD *) GetTagData(WHEEL_Donation      , NULL          , msg->ops_AttrList);
	    data->maxpens  =           GetTagData(WHEEL_MaxPens       , 256	      , msg->ops_AttrList);
	    
    	    colorwheel_set(cl, o, msg);
	} else {
	    CoerceMethod(cl, o, OM_DISPOSE);
	    o = NULL;
	}
    	   
    }
    ReturnPtr ("ColorWheel::New", Object *, o);
}

/***************************************************************************************************/

STATIC IPTR colorwheel_get(Class *cl, Object *o, struct opGet *msg)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);
    IPTR 			retval = 1UL;
   
    if (data->gradobj)
    {
        IPTR gradval;
	
	GetAttr(GRAD_CurVal, data->gradobj, &gradval);
	gradval = 0xFFFF - gradval;
	
	data->hsb.cw_Brightness = ((ULONG)gradval) * 0x10000 + (ULONG)gradval;
	ConvertHSBToRGB(&data->hsb, &data->rgb);
    }
    
    switch(msg->opg_AttrID)
    {
        case WHEEL_Hue:
	    *msg->opg_Storage = (IPTR)data->hsb.cw_Hue;
	    break;
	    
	case WHEEL_Saturation:
	    *msg->opg_Storage = (IPTR)data->hsb.cw_Saturation;
	    break;
	    
	case WHEEL_Brightness:
	    *msg->opg_Storage = (IPTR)data->hsb.cw_Brightness;
	    break;
	    
	case WHEEL_HSB:
	    *(struct ColorWheelHSB *)msg->opg_Storage = data->hsb;
	    break;
	    
	case WHEEL_Red:
	    *msg->opg_Storage = (IPTR)data->rgb.cw_Red;
	    break;
	     
	case WHEEL_Green:
	    *msg->opg_Storage = (IPTR)data->rgb.cw_Green;
	    break;
	    
	case WHEEL_Blue:
	    *msg->opg_Storage = (IPTR)data->rgb.cw_Blue;
	    break;
	    
	case WHEEL_RGB:
	    *(struct ColorWheelRGB *)msg->opg_Storage = data->rgb;
	    break;
	            
	default:
	    retval = DoSuperMethodA(cl, o, (Msg)msg);
	    break;
    }
   
    return retval;
}

/***************************************************************************************************/

STATIC VOID colorwheel_render(Class *cl, Object *o, struct gpRender *msg)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);    
    struct DrawInfo 		*dri = msg->gpr_GInfo->gi_DrInfo;
    struct RastPort 		*rp = msg->gpr_RPort;
    struct IBox			gbox;
    LONG			redraw = msg->gpr_Redraw;
    
    EnterFunc(bug("ColorWheel::Render()\n"));    

    GetGadgetIBox(o, msg->gpr_GInfo, &gbox);
    data->dri = dri;
    
    if (!data->bm || (data->bmwidth != gbox.Width) || (data->bmheight != gbox.Height))
    {
        redraw = GREDRAW_REDRAW;
    }
    
    switch (redraw)
    {
    	case GREDRAW_REDRAW:
    	    RenderWheel(data, rp, &gbox, ColorWheelBase);
	    RenderKnob(data, rp, &gbox, FALSE, ColorWheelBase);
	    break;
	    
    	case GREDRAW_UPDATE:    	 
	    RenderKnob(data, rp, &gbox, TRUE, ColorWheelBase);
    	    break;
    	    
    	    
    } /* switch (redraw method) */
    
    if (EG(o)->Flags & GFLG_DISABLED)
    {
    	DrawDisabledPattern(rp, &gbox, dri->dri_Pens[SHADOWPEN], ColorWheelBase);
    }
        	
    ReturnVoid("ColorWheel::Render");
}

/***************************************************************************************************/

STATIC VOID colorwheel_dispose(Class *cl, Object *o, Msg msg)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);
    
    if (data->rgblinebuffer) FreeVec(data->rgblinebuffer);
    if (data->bm)
    {
        WaitBlit();
	FreeBitMap(data->bm);
    }
    DoSuperMethodA(cl, o, msg);
}

/***************************************************************************************************/

STATIC IPTR colorwheel_hittest(Class *cl, Object *o, struct gpHitTest *msg)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);
    ULONG			hue, sat;
    IPTR 			retval = 0UL;

    EnterFunc(bug("ColorWheel::HitTest()\n"));
    
    if (data->wheeldrawn)
    {
        WORD mousex = msg->gpht_Mouse.X - (data->wheelcx - data->wheelrx);
	WORD mousey = msg->gpht_Mouse.Y - (data->wheelcy - data->wheelry);
	
	kprintf("checking %d,%d %d,%d,\n", mousex, mousey, data->wheelrx, data->wheelry);
	
	if (CalcWheelColor(mousex,
			   mousey,
			   (DOUBLE)data->wheelrx,
			   (DOUBLE)data->wheelry,
			   &hue,
			   &sat))
	{
	    retval = GMR_GADGETHIT;
	}
    }
    
    ReturnInt("ColorWheel::HitTest", IPTR, retval);
}

/***************************************************************************************************/

STATIC IPTR colorwheel_goactive(Class *cl, Object *o, struct gpInput *msg)
{
    struct ColorWheelData 	*data = INST_DATA(cl, o);
    IPTR 			retval = GMR_NOREUSE;

    EnterFunc(bug("ColorWheel::GoActive()\n"));
    
    if (data->wheeldrawn && msg->gpi_IEvent)
    {
    } /* if (data->wheeldrawn && msg->gpi_IEvent) */
    
    ReturnInt("ColorWheel::GoActive", IPTR, retval);
}

/***************************************************************************************************/

STATIC IPTR colorwheel_handleinput(Class *cl, Object *o, struct gpInput *msg)
{
    struct ColorWheelData	*data = INST_DATA(cl, o);
    IPTR 			retval = 0UL;
    struct InputEvent 		*ie = msg->gpi_IEvent;
    
    EnterFunc(bug("ColorWheel::HandleInput \n"));

    retval = GMR_MEACTIVE;
        
    ReturnInt("ColorWheel::HandleInput", IPTR, retval);
}

/***************************************************************************************************/

AROS_UFH3S(IPTR, dispatch_colorwheelclass,
    AROS_UFHA(Class *,  cl,  A0),
    AROS_UFHA(Object *, o,   A2),
    AROS_UFHA(Msg,      msg, A1)
)
{
    IPTR retval = 0UL;
    
    switch(msg->MethodID)
    {
	case OM_NEW:
	    retval = (IPTR)colorwheel_new(cl, o, (struct opSet *)msg);
	    break;
	
	case OM_DISPOSE:
	    colorwheel_dispose(cl, o, msg);
	    break;
	    
	case GM_RENDER:
	    colorwheel_render(cl, o, (struct gpRender *)msg);
	    break;
	
	case GM_HITTEST:
	    retval = colorwheel_hittest(cl, o, (struct gpHitTest *)msg);
	    break;
	    
	case GM_GOACTIVE:
	    retval = colorwheel_goactive(cl, o, (struct gpInput *)msg);
	    break;

	case GM_HANDLEINPUT:
	    retval = colorwheel_handleinput(cl, o, (struct gpInput *)msg);
	    break;

	case OM_SET:
	case OM_UPDATE:
	    retval = colorwheel_set(cl, o, (struct opSet *)msg);
	    break;

	case OM_GET:
	    retval = colorwheel_get(cl, o, (struct opGet *)msg);
	    break;
	    
	default:
	    retval = DoSuperMethodA(cl, o, msg);
	    break;
	    
    } /* switch */

    return (retval);
    
}  /* dispatch_colorwheelclass */


#undef ColorWheelBase

/***************************************************************************************************/

struct IClass *InitColorWheelClass (struct ColorWheelBase_intern * ColorWheelBase)
{
    struct IClass *cl = NULL;

    if ((cl = MakeClass("colorwheel.gadget", GADGETCLASS, NULL, sizeof(struct ColorWheelData), 0)))
    {
	cl->cl_Dispatcher.h_Entry    = (APTR)AROS_ASMSYMNAME(dispatch_colorwheelclass);
	cl->cl_Dispatcher.h_SubEntry = NULL;
	cl->cl_UserData 	     = (IPTR)ColorWheelBase;

	AddClass (cl);
    }

    return (cl);
}

/***************************************************************************************************/
