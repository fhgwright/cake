/*
   (C) 1997 AROS - The Amiga Replacement OS
   $Id$

   Desc: GadTools gadget creation functions
   Lang: English
*/
#include <stdio.h>
#include <proto/exec.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/screens.h>
#include <intuition/icclass.h>
#include <proto/utility.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>
#include <gadgets/aroscheckbox.h>
#include <gadgets/aroscycle.h>
#include <gadgets/arosmx.h>
#include <gadgets/arospalette.h>
#include "gadtools_intern.h"


/*******************
**  makebutton()  **
*******************/
struct Gadget *makebutton(struct GadToolsBase_intern *GadToolsBase,
			  struct TagItem stdgadtags[],
			  struct VisualInfo *vi,
			  struct TagItem *taglist)
{
    struct Gadget *obj;
    Class *cl;
    struct TagItem tags[] =
    {
	{GA_Disabled, FALSE},
	{GA_Immediate, FALSE},
	{GA_RelVerify, TRUE},
	{TAG_MORE, (IPTR) NULL}
    };

    cl = makebuttonclass(GadToolsBase);
    if (!cl)
	return NULL;

    tags[0].ti_Data = GetTagData(GA_Disabled, FALSE, taglist);
    tags[1].ti_Data = GetTagData(GA_Immediate, FALSE, taglist);
    tags[3].ti_Data = (IPTR) stdgadtags;

    obj = (struct Gadget *) NewObjectA(cl, NULL, tags);

    return obj;
}

/*********************
**  makecheckbox()  **
*********************/
struct Gadget *makecheckbox(struct GadToolsBase_intern *GadToolsBase,
			    struct TagItem stdgadtags[],
			    struct VisualInfo *vi,
			    struct TagItem *taglist)
{
    struct Gadget *obj;
    struct TagItem tags[] =
    {
	{GA_Disabled, FALSE},
	{GTCB_Checked, FALSE},
	{TAG_MORE, (IPTR) NULL}
    };

    if (!GadToolsBase->aroscbbase)
        GadToolsBase->aroscbbase = OpenLibrary("SYS:Classes/Gadgets/aroscheckbox.gadget", 0);
    if (!GadToolsBase->aroscbbase)
        return NULL;

    tags[0].ti_Data = GetTagData(GA_Disabled, FALSE, taglist);
    tags[1].ti_Data = GetTagData(GTCB_Checked, FALSE, taglist);
    tags[2].ti_Data = (IPTR) stdgadtags;

    if (!GetTagData(GTCB_Scaled, FALSE, taglist)) {
	stdgadtags[TAG_Width].ti_Data = CHECKBOX_WIDTH;
	stdgadtags[TAG_Height].ti_Data = CHECKBOX_HEIGHT;
    }
    obj = (struct Gadget *) NewObjectA(NULL, AROSCHECKBOXCLASS, tags);

    return obj;
}

/******************
**  makecycle()  **
******************/
struct Gadget *makecycle(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TagItem *taglist)
{
    struct Gadget *obj;
    struct TagItem tags[] =
    {
	{GA_Disabled, FALSE},
	{GTCY_Labels, FALSE},
        {GTCY_Active, 0},
	{TAG_MORE, (IPTR) NULL}
    };

    if (!GadToolsBase->aroscybase)
        GadToolsBase->aroscybase = OpenLibrary("SYS:Classes/Gadgets/aroscycle.gadget", 0);
    if (!GadToolsBase->aroscybase)
        return NULL;

    tags[0].ti_Data = GetTagData(GA_Disabled, FALSE, taglist);
    tags[1].ti_Data = GetTagData(GTCY_Labels, FALSE, taglist);
    tags[2].ti_Data = GetTagData(GTCY_Active, 0, taglist);
    tags[3].ti_Data = (IPTR) stdgadtags;

    obj = (struct Gadget *) NewObjectA(NULL, AROSCYCLECLASS, tags);

    return obj;
}

/***************
**  makemx()  **
***************/
struct Gadget *makemx(struct GadToolsBase_intern *GadToolsBase,
		      struct TagItem stdgadtags[],
		      struct VisualInfo *vi,
		      struct TagItem *taglist)
{
    struct Gadget *gad;
    int labels = 0;
    STRPTR *labellist;
    struct TagItem *tag, tags[] =
    {
	{GA_Disabled, FALSE},
	{AROSMX_Labels, (IPTR) NULL},
	{AROSMX_Active, 0},
	{AROSMX_Spacing, 1},
        {AROSMX_TickHeight, MX_HEIGHT},
        {AROSMX_TickLabelPlace, GV_LabelPlace_Right},
	{TAG_MORE, (IPTR) NULL}
    };

    if (!GadToolsBase->arosmxbase)
        GadToolsBase->arosmxbase = OpenLibrary("SYS:Classes/Gadgets/arosmutualexclude.gadget", 0);
    if (!GadToolsBase->arosmxbase)
        return NULL;

    tags[0].ti_Data = GetTagData(GA_Disabled, FALSE, taglist);
    labellist = (STRPTR *) GetTagData(GTMX_Labels, (IPTR) NULL, taglist);
    if (!labellist)
	return NULL;
    tags[1].ti_Data = (IPTR) labellist;
    tags[2].ti_Data = GetTagData(GTMX_Active, 0, taglist);
    tags[3].ti_Data = GetTagData(GTMX_Spacing, 1, taglist);
    if (GetTagData(GTMX_Scaled, FALSE, taglist))
        tags[4].ti_Data = stdgadtags[TAG_Height].ti_Data;
    else
        stdgadtags[TAG_Width].ti_Data = MX_WIDTH;
    switch (stdgadtags[TAG_LabelPlace].ti_Data & 0x1f)
    {
    case PLACETEXT_LEFT:
        tags[5].ti_Data = GV_LabelPlace_Left;
        break;
    case PLACETEXT_ABOVE:
        tags[5].ti_Data = GV_LabelPlace_Above;
        break;
    case PLACETEXT_BELOW:
        tags[5].ti_Data = GV_LabelPlace_Below;
        break;
    }
    tags[6].ti_Data = (IPTR) stdgadtags;

    tag = FindTagItem(GTMX_TitlePlace, taglist);
    if (tag)
    {
        switch (tag->ti_Data)
        {
        case PLACETEXT_LEFT:
            stdgadtags[TAG_LabelPlace].ti_Data = GV_LabelPlace_Left;
            break;
        case PLACETEXT_RIGHT:
            stdgadtags[TAG_LabelPlace].ti_Data = GV_LabelPlace_Right;
            break;
        case PLACETEXT_ABOVE:
            stdgadtags[TAG_LabelPlace].ti_Data = GV_LabelPlace_Above;
            break;
        case PLACETEXT_BELOW:
            stdgadtags[TAG_LabelPlace].ti_Data = GV_LabelPlace_Below;
            break;
        default:
            freeitext(GadToolsBase,
                      (struct IntuiText *)stdgadtags[TAG_IText].ti_Data);
            stdgadtags[TAG_IText].ti_Data = (IPTR)NULL;
            break;
        }
    } else
    {
        freeitext(GadToolsBase,
                  (struct IntuiText *)stdgadtags[TAG_IText].ti_Data);
        stdgadtags[TAG_IText].ti_Data = (IPTR)NULL;
    }

    while (labellist[labels])
	labels++;

    gad = (struct Gadget *) NewObjectA(NULL, AROSMXCLASS, tags);

    return gad;
}

/*******************
**  makepalette() **
*******************/
struct Gadget *makepalette(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TagItem *taglist)
{
    struct Gadget *obj = NULL;

    struct TagItem *tag, tags[] =
    {
    	{GA_RelVerify,		TRUE},
	{GA_Disabled,		FALSE},
	{GTPA_Depth,		1},
        {GTPA_Color,		0},
        {GTPA_ColorOffset,	0},
        {GTPA_IndicatorWidth,	0},
        {GTPA_IndicatorHeight,	0},
        {GTPA_NumColors,	0},
        {GTPA_ColorTable,	0},
	{TAG_MORE, (IPTR) NULL}
    };
    
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GA_Disabled:		tags[1].ti_Data = tidata; break;
    	case GTPA_Depth:		tags[2].ti_Data	= tidata; break;
    	case GTPA_Color:		tags[3].ti_Data	= tidata; break;
    	case GTPA_ColorOffset:		tags[4].ti_Data	= tidata; break;
    	case GTPA_IndicatorWidth:	tags[5].ti_Data	= tidata; break;
    	case GTPA_IndicatorHeight:	tags[6].ti_Data	= tidata; break;
    	case GTPA_NumColors:		tags[7].ti_Data	= tidata; break;
    	case GTPA_ColorTable:		tags[8].ti_Data	= tidata; break;
    	    
    	} /* switch() */
    	
    } /* while (iterate taglist) */

    tags[9].ti_Data = (IPTR)stdgadtags;

    if (!GadToolsBase->arospabase)
        GadToolsBase->arospabase = OpenLibrary(AROSPALETTECLASSPATH, 0);
    if (!GadToolsBase->arospabase)
        return NULL;

    obj = (struct Gadget *) NewObjectA(NULL, AROSPALETTECLASS, tags);

    return obj;
}

/*****************
**  maketext()  **
*****************/
struct Gadget *maketext(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{
    struct Gadget *obj = NULL;
    Class *cl;
    
    struct TagItem *tag, tags[] =
    {
    	{GTTX_Text,		(IPTR)"Blah"},
    	{GTTX_CopyText,		FALSE},
    	{GTTX_Clipped,		FALSE},
    	{GTTX_Border,		FALSE},
    	{GTTX_FrontPen,		TEXTPEN},
    	{GTTX_BackPen,		BACKGROUNDPEN},
    	{GTTX_Justification,	GTJ_LEFT},
    	{GTA_Text_Format,	(IPTR)"%s"},
    	{GA_TextAttr,		(IPTR)NULL},
	{TAG_MORE, (IPTR) NULL}
    };

    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GTTX_Text:			tags[0].ti_Data = tidata; break;
    	case GTTX_CopyText:		tags[1].ti_Data	= tidata; break;
    	case GTTX_Clipped:		tags[2].ti_Data	= tidata; break;
    	case GTTX_Border:		tags[3].ti_Data	= tidata; break;
    	case GTTX_FrontPen:		tags[4].ti_Data	= tidata; break;
    	case GTTX_BackPen:		tags[5].ti_Data	= tidata; break;
    	case GTTX_Justification:	tags[6].ti_Data	= tidata; break;
    	    
    	}
    	
    } /* while (iterate taglist) */

    /* Be sure not to pass GA_TextAttr, NULL */
    if (tattr)
    	tags[8].ti_Data = (IPTR)tattr;
    else
    	tags[8].ti_Tag = TAG_IGNORE;
    tags[9].ti_Data = (IPTR)stdgadtags;

    cl = maketextclass(GadToolsBase);
    if (!cl)
    	return (NULL);
    obj = (struct Gadget *) NewObjectA(cl, NULL, tags);

    return (obj);
}

/*******************
**  makenumber()  **
*******************/
struct Gadget *makenumber(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{
    struct Gadget *obj = NULL;
    Class *cl;

    struct TagItem *tag, tags[] =
    {
    	{GTNM_Number,		0},
    	{GTNM_Format,		(IPTR)"%ld"},
    	{GTNM_Clipped,		FALSE},
    	{GTNM_Border,		FALSE},
    	{GTNM_FrontPen,		TEXTPEN},
    	{GTNM_BackPen,		BACKGROUNDPEN},
    	{GTNM_Justification,	GTJ_CENTER},
    	{GTNM_MaxNumberLen,	100},
    	{GA_TextAttr,		(IPTR)NULL},
	{TAG_MORE, (IPTR) NULL}
    };
    
   
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GTNM_Number:		tags[0].ti_Data = tidata; break;
    	case GTNM_Format:		tags[1].ti_Data	= tidata; break;
    	case GTNM_Clipped:		tags[2].ti_Data	= tidata; break;
    	case GTNM_Border:		tags[3].ti_Data	= tidata; break;
    	case GTNM_FrontPen:		tags[4].ti_Data	= tidata; break;
    	case GTNM_BackPen:		tags[5].ti_Data	= tidata; break;
    	case GTNM_Justification:	tags[6].ti_Data	= tidata; break;
    	case GTNM_MaxNumberLen:		tags[7].ti_Data	= tidata; break;
    	    
    	}
    	
    } /* while (iterate taglist) */

    /* Be sure not to pass GA_TextAttr, NULL */
    if (tattr)
    	tags[8].ti_Data = (IPTR)tattr;
    else
    	tags[8].ti_Tag = TAG_IGNORE;
    	
    tags[9].ti_Data = (IPTR)stdgadtags;

    cl = maketextclass(GadToolsBase);
    if (!cl)
    	return (NULL);
    obj = (struct Gadget *) NewObjectA(cl, NULL, tags);

    return  (obj);
}


/*******************
**  makeslider()  **
*******************/

/* This MUST be global, since the gadgetclass doesn't copy ICA_MAPPINGs */
const struct TagItem slider2level[] =
{
    {GTSL_Level,	GTNM_Number},
    {TAG_DONE }
};

struct Gadget *makeslider(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{

    struct TagItem *tag;
    
    struct TagItem stags[] =
    {
    	{GA_Disabled,	FALSE},
    	{GA_RelVerify,	FALSE},
    	{GA_Immediate,	FALSE},
    	{GTSL_Min,	0},
    	{GTSL_Max,	15},
    	{GTSL_Level,	0},
    	{PGA_Freedom,	FREEHORIZ},
    	{TAG_MORE,	(IPTR)NULL}
    };
    
    struct TagItem ltags[] = 
    {
    	 {GA_Left,	0},
    	 {GA_Top,	0},
    	 {GA_Width,	0},
    	 {GA_Height,	0},
    	 {GA_TextAttr,	(IPTR)NULL},
    	 {GTNM_Format,	(IPTR)NULL},
    	 {GTNM_Justification, GTJ_LEFT},
	 {GTA_Text_DispFunc,	(IPTR)NULL},
	 {GA_Previous,	(IPTR)NULL},
	 {GA_DrawInfo,	(IPTR)NULL},
	 {GTNM_Number,	0},
	 {TAG_DONE,}
    };
    STRPTR lformat = NULL;
    WORD lmaxlen = 0;
    LONG lmaxpixellen = 0L;
    UBYTE lplace = PLACETEXT_LEFT;
    WORD level = 0;
    
    Class *slidercl;
    struct Gadget *slidergad = NULL, *levelgad = NULL;

    /* open neede classes */
    slidercl = makesliderclass(GadToolsBase);
    if (!slidercl)
    	return (NULL);

    	
    /* Parse tags */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	    
    	switch (tag->ti_Tag)
    	{
    	
    	/* Slider tags */
    	case GA_Disabled:	stags[0].ti_Data = tidata; break;
    	case GA_RelVerify:	stags[1].ti_Data = tidata; break;
    	case GA_Immediate:	stags[2].ti_Data = tidata; break;
    	case GTSL_Min:		stags[3].ti_Data = tidata; break;
    	case GTSL_Max:		stags[4].ti_Data = tidata; break;
    	case GTSL_Level:	level = stags[5].ti_Data = tidata; break;
    	case PGA_Freedom:
    	    if (tidata == LORIENT_HORIZ)
    	    	stags[6].ti_Data = FREEHORIZ;
    	    else
    	    	stags[6].ti_Data = FREEVERT;
    	    break;
    	
    	/* Level tags */    
    	case GTSL_LevelFormat:   lformat          = (STRPTR)tidata;	break;
    	case GTSL_MaxLevelLen:   lmaxlen          = (UWORD)tidata;	break;
	case GTSL_MaxPixelLen:   lmaxpixellen     = (ULONG)tidata; 	break;
	case GTSL_LevelPlace:    lplace	      	  = (UBYTE)tidata;	break;
    	case GTSL_Justification: ltags[6].ti_Data = tidata;	  	break;
    	case GTSL_DispFunc:	 ltags[7].ti_Data = tidata;  		break;

    	} 
    	    
    } /* while (iterate taglist) */
    
    /* Create slider gadget */
    stags[7].ti_Data = (IPTR)stdgadtags;
    slidergad = NewObjectA(slidercl, NULL, stags);
    if (!slidergad)
    	return (NULL);

    if (lformat || lmaxlen || lmaxpixellen)
    {
	WORD x, y;
	UWORD ysize;
	Class *textcl;
	
	struct TagItem lntags[] =
	{
	     {ICA_TARGET,	(IPTR)NULL},
	     {ICA_MAP,		(IPTR)NULL},
	     {TAG_DONE,}
	};
	    
    	/* Set som defaults */
    	if (!lformat)
    	    lformat = "%ld";
    	if (!lmaxlen)
    	    lmaxlen = 2;
    	if (!lmaxpixellen)
    	{
    	    struct TextFont *font;
    	    UWORD xsize;
    	    
    	    ysize = vi->vi_dri->dri_Font->tf_YSize;
    	    xsize = vi->vi_dri->dri_Font->tf_XSize;

	    if (tattr)
	    {
    	    	font = OpenFont(tattr);
    	    	if (font)
    	   	{
    	   	    ysize = font->tf_YSize;
    	   	    xsize = font->tf_XSize;
    	   	    CloseFont(font);
    	   	}
    	   	else /* If no valid tattr */
    	   	    ltags[4].ti_Tag = TAG_IGNORE;
    	    }
    	    else /* If no valid tattr */
    	        ltags[4].ti_Tag = TAG_IGNORE;

    	    lmaxpixellen = lmaxlen * xsize;
    	        
    	    ltags[4].ti_Data = (ULONG)tattr;
    	    	
    	} /* if (!lmaxpixellen) */
    	
    	switch (lplace)
    	{
    	    case PLACETEXT_LEFT:
            	x = slidergad->LeftEdge - lmaxpixellen - 4;
            	y = slidergad->TopEdge + (slidergad->Height - ysize) / 2 + 1;
    	    	break;
    	    case PLACETEXT_RIGHT:
            	x = slidergad->LeftEdge + slidergad->Width + 5;
            	y = slidergad->TopEdge  + (slidergad->Height - ysize) / 2 + 1;
    	    	break;
    	    case PLACETEXT_ABOVE:
            	x = slidergad->LeftEdge - (lmaxpixellen - slidergad->Width) / 2;
            	y = slidergad->TopEdge  - ysize - 2;
    	    	break;
    	    case PLACETEXT_BELOW:
            	x = slidergad->LeftEdge - (lmaxpixellen - slidergad->Width) / 2;
            	y = slidergad->TopEdge  + slidergad->Height + 3;
 	    	break;
    	}

        /* Create the levelobj */
    	
    	ltags[0].ti_Data = (IPTR)x;
    	ltags[1].ti_Data = (IPTR)y;
    	ltags[2].ti_Data = (IPTR)lmaxpixellen;
    	ltags[3].ti_Data = (IPTR)ysize;
    	ltags[5].ti_Data = (IPTR)lformat;
    	ltags[8].ti_Data = (IPTR)slidergad;
    	ltags[9].ti_Data = (IPTR)vi->vi_dri;
    	ltags[10].ti_Data = (IPTR)level;

	textcl = maketextclass(GadToolsBase);
    	if (!textcl)
    	{
    	    DisposeObject((Object *)slidergad);
    	    return (NULL);
    	}
    	    
    	levelgad = (struct Gadget *)NewObjectA(textcl, NULL, ltags);
    	if (!levelgad)
    	{
    	    DisposeObject((Object *)slidergad);
    	    return (NULL);
    	}
    	
    	/* Set up a notification from the slider to the level */
    	lntags[0].ti_Data = (IPTR)levelgad;
    	lntags[1].ti_Data = (IPTR)slider2level;
    	SetAttrsA((Object *)slidergad, lntags);
    	    
    } /* if (slider should have a level attached) */
    
    return (slidergad);

}                         

#ifdef SDEBUG
#   undef SDEBUG
#endif
#ifdef DEBUG
#   undef DEBUG
#endif
#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

/*********************
**  makescroller()  **
*********************/

const struct TagItem arrow_dec2scroller[] =
{
    {GA_ID,	GTA_Scroller_Dec},
    {TAG_DONE, }
};

const struct TagItem arrow_inc2scroller[] =
{
    {GA_ID,	GTA_Scroller_Inc},
    {TAG_DONE, }
};


struct Gadget *makescroller(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TagItem *taglist)
{
    struct Gadget *scroller = NULL,
    		  *arrow_dec = NULL,
    		  *arrow_inc = NULL ;
    Class *cl;

    struct TagItem *tag, stags[] =
    {
    	{GTSC_Top,	0},
    	{GTSC_Total,	0},
    	{GTSC_Visible,	2},
    	{PGA_Freedom,	FREEHORIZ},
    	{GA_Disabled,	FALSE},
    	{GA_RelVerify,	FALSE},
    	{GA_Immediate,	FALSE},
	{TAG_MORE, (IPTR) NULL}
    };
    
    struct TagItem *scr_dim_tagitem;
    
    UWORD freedom = stags[3].ti_Data; /* default */
    WORD arrowdim;
    BOOL relverify, immediate;
    ULONG scr_dim_tag;
    
    EnterFunc(bug("makescroller(stdgadtags=%p, vi=%p, taglist = %p)\n",
    		stdgadtags, vi, taglist));
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GTSC_Top:		stags[0].ti_Data = tidata; break;
    	case GTSC_Total:	stags[1].ti_Data = tidata; break;
    	case GTSC_Visible:	stags[2].ti_Data = tidata; break;
    	case PGA_Freedom:
    	    if (tidata == LORIENT_HORIZ)
    	    	freedom = stags[3].ti_Data = FREEHORIZ;
    	    else
    	    	freedom = stags[3].ti_Data = FREEVERT;
    	    break;
    	case GA_Disabled:	stags[4].ti_Data = tidata; break;
    	case GA_RelVerify:	relverify = stags[5].ti_Data = tidata; break;
    	case GA_Immediate:	immediate = stags[6].ti_Data = tidata; break;
    	
    	case GTSC_Arrows:	arrowdim = (WORD)tidata;
    	    
    	}
    	
    } /* while (iterate taglist) */

    stags[7].ti_Data = (IPTR)stdgadtags;
    
    /* Substract the arrow's total size from the sroller's size */
    scr_dim_tag = ((freedom == FREEVERT) ? GA_Height : GA_Width);
    
    scr_dim_tagitem = FindTagItem(scr_dim_tag, stdgadtags);
    scr_dim_tagitem->ti_Data -= 2 * arrowdim;

    cl = makescrollerclass(GadToolsBase);
    if (!cl)
    	return (NULL);
    scroller = (struct Gadget *) NewObjectA(cl, NULL, stags);

    if (!scroller)
    	return (NULL);
    
    if (arrowdim) /* Scroller has arroes ? */
    {
    	Class *arrowcl;
    	struct TagItem antags[] =
    	{
    	    {ICA_TARGET,	(IPTR)NULL},
    	    {ICA_MAP,		(IPTR)NULL},
    	    {TAG_DONE,}
    	};
    	
    	struct TagItem atags[] =
    	{
    	    {GA_Left,		0},
    	    {GA_Top,		0},
    	    {GA_Width,		0},
    	    {GA_Height,		0},
    	    {GTA_Arrow_Type,	0},
    	    {GA_DrawInfo,	(IPTR)NULL},
    	    {GA_Previous,	(IPTR)NULL},
    	    {GTA_Arrow_Scroller, (IPTR)NULL},
    	    {GA_RelVerify,	0},
    	    {GA_Immediate,	0},
    	    {GA_ID,		0},
    	    {TAG_DONE}
    	};
    	
    	atags[5].ti_Data = (IPTR)vi->vi_dri;    /* Set GA_DrawInfo */
    	atags[6].ti_Data = (IPTR)scroller;	/* Set GA_Previous */
    	atags[7].ti_Data = (IPTR)scroller;	/* Set GTA_Arrow_Scroller */

    	/* These must be the same as for scroller */
    	atags[8].ti_Data = (IPTR)relverify;
    	atags[9].ti_Data = (IPTR)immediate;
    	atags[10].ti_Data = (IPTR)GetTagData(GA_ID, 0, stdgadtags); 
    	
    	/* Open needed class */
    	arrowcl = makearrowclass(GadToolsBase);
    	if (!arrowcl)
    	    goto failure;
    	    
    	if (freedom == FREEVERT)
    	{
    	    D(bug("Freedom=FREEVERT\n"));
    	    atags[0].ti_Data = scroller->LeftEdge;
    	    atags[1].ti_Data = scroller->TopEdge + scroller->Height;
    	    atags[2].ti_Data = scroller->Width;
    	    atags[3].ti_Data = arrowdim;
    	    atags[4].ti_Data = UPIMAGE;
    	    
    	    arrow_dec = NewObjectA(arrowcl, NULL, atags);
    	    if (!arrow_dec)
    	    	goto failure;
    	    
    	    ((ULONG)atags[1].ti_Data) += arrowdim;
    	    atags[4].ti_Data = DOWNIMAGE;
    	    atags[6].ti_Data = (IPTR)arrow_dec;
	
    	    arrow_inc = NewObjectA(arrowcl, NULL, atags);
    	    if (!arrow_inc)
    	    	goto failure;
    	    
    	}
    	else
    	{
    	    D(bug("Freedom=FREEHORIZ\n"));

    	    atags[0].ti_Data = scroller->LeftEdge + scroller->Width;;
    	    atags[1].ti_Data = scroller->TopEdge;
    	    atags[2].ti_Data = arrowdim;
    	    atags[3].ti_Data = scroller->Height;
    	    atags[4].ti_Data = LEFTIMAGE;
    	    
    	    arrow_dec = NewObjectA(arrowcl, NULL, atags);
    	    if (!arrow_dec)
    	    	goto failure;
    	    
    	    ((ULONG)atags[0].ti_Data) += arrowdim;
    	    atags[4].ti_Data = RIGHTIMAGE;
    	    atags[6].ti_Data = (IPTR)arrow_dec;

    	    arrow_inc = NewObjectA(arrowcl, NULL, atags);
    	    if (!arrow_inc)
    	    	goto failure;
    	    	
    	    	
    	} /* if (scroller is FREEVERT or FREEHORIZ) */

    	
    	/* Create notfications from arrows to scroller */
    	antags[0].ti_Data = (IPTR)scroller;

    	antags[1].ti_Data = (IPTR)arrow_dec2scroller;    	
    	SetAttrsA((Object *)arrow_dec, antags);

    	antags[1].ti_Data = (IPTR)arrow_inc2scroller;
    	SetAttrsA((Object *)arrow_inc, antags);
    	
    } /* if (scroller should have arrows attached) */
    
    ReturnPtr ("makescroller", struct Gadget *, scroller);
    
failure:
   if (scroller)
   	DisposeObject((Object *)scroller);

   if (arrow_dec)
   	DisposeObject((Object *)arrow_dec);

   if (arrow_inc)
   	DisposeObject((Object *)arrow_inc);
   
   ReturnPtr("makescroller", struct Gadget *, NULL);
}

/*******************
**  makestring()  **
*******************/
struct Gadget *makestring(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{
    struct Gadget *obj = NULL;
    Class *cl;

    struct TagItem *tag, tags[] =
    {
    	{GA_Disabled,		FALSE},
    	{GA_Immediate,		FALSE},
    	{GA_TabCycle,		FALSE},
    	{GTST_String,		(IPTR)NULL},
    	{GTST_MaxChars,		0UL},
    	{GTST_EditHook,		(IPTR)NULL},
    	{STRINGA_ExitHelp,	FALSE},
    	{STRINGA_Justification,	GACT_STRINGLEFT},
    	{STRINGA_ReplaceMode,	FALSE},
    	{GA_TextAttr,		(IPTR)NULL},
	{TAG_MORE, 	(IPTR)NULL}
    };
    
   
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GA_Disabled:		tags[0].ti_Data = tidata; break;
    	case GA_Immediate:		tags[1].ti_Data	= tidata; break;
    	case GA_TabCycle:		tags[2].ti_Data	= tidata; break;
    	case GTST_String:		tags[3].ti_Data	= tidata; break;
    	case GTST_MaxChars:		tags[4].ti_Data	= tidata; break;
    	case GTST_EditHook:		tags[5].ti_Data	= tidata; break;
    	case STRINGA_ExitHelp:		tags[6].ti_Data	= tidata; break;
    	case STRINGA_Justification:	tags[7].ti_Data	= tidata; break;
    	case STRINGA_ReplaceMode:	tags[8].ti_Data	= tidata; break;
    	}
    	
    } /* while (iterate taglist) */
    
    if (tattr) /* Text Attr supplied ? */
	tags[9].ti_Data = (IPTR)tattr; 
    else
    	tags[9].ti_Tag = TAG_IGNORE; /* Don't pass GA_TextAttr, NULL */
	
    tags[10].ti_Data = (IPTR)stdgadtags;

    cl = makestringclass(GadToolsBase);
    if (!cl)
    	return (NULL);
    obj = (struct Gadget *) NewObjectA(cl, NULL, tags);

    return  (obj);
}

/********************
**  makeinteger()  **
********************/
struct Gadget *makeinteger(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{
    struct Gadget *obj = NULL;
    Class *cl;

    struct TagItem *tag, tags[] =
    {
    	{GA_Disabled,		FALSE},
    	{GA_Immediate,		FALSE},
    	{GA_TabCycle,		FALSE},
    	{GTIN_Number,		0L},
    	{GTIN_MaxChars,		10L},
    	{GTIN_EditHook,		(IPTR)NULL},
    	{STRINGA_ExitHelp,	FALSE},
    	{STRINGA_Justification,	GACT_STRINGLEFT},
    	{STRINGA_ReplaceMode,	FALSE},
    	{GA_TextAttr,		(IPTR)NULL},
	{TAG_MORE, 	(IPTR)NULL}
    };
    
   
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GA_Disabled:		tags[0].ti_Data = tidata; break;
    	case GA_Immediate:		tags[1].ti_Data	= tidata; break;
    	case GA_TabCycle:		tags[2].ti_Data	= tidata; break;
    	case GTIN_Number:		tags[3].ti_Data	= tidata; break;
    	case GTIN_MaxChars:		tags[4].ti_Data	= tidata; break;
    	case GTIN_EditHook:		tags[5].ti_Data	= tidata; break;
    	case STRINGA_ExitHelp:		tags[6].ti_Data	= tidata; break;
    	case STRINGA_Justification:	tags[7].ti_Data	= tidata; break;
    	case STRINGA_ReplaceMode:	tags[8].ti_Data	= tidata; break;
    	}
    	
    } /* while (iterate taglist) */

    if (tattr) /* Text Attr supplied ? */
	tags[9].ti_Data = (IPTR)tattr; 
    else
    	tags[9].ti_Tag = TAG_IGNORE; /* Don't pass GA_TextAttr, NULL */

    tags[10].ti_Data = (IPTR)stdgadtags;

    cl = makestringclass(GadToolsBase);
    if (!cl)
    	return (NULL);
    obj = (struct Gadget *) NewObjectA(cl, NULL, tags);

    return  (obj);
}


/*********************
**  makelistview()  **
*********************/

const struct TagItem scroller2lv[] =
{
    {PGA_Top,	GTLV_Top},
    {TAG_DONE, }
};

/* Spacig between scroller and listview */
#define SCROLLER_SPACING 2


struct Gadget *makelistview(struct GadToolsBase_intern *GadToolsBase,
                         struct TagItem stdgadtags[],
                         struct VisualInfo *vi,
                         struct TextAttr *tattr,
                         struct TagItem *taglist)
{
    struct Gadget *lvgad = NULL,  *showselgad = NULL, *scrollergad; /* important to set these to NULL */
    Class *cl;


    WORD scroller_width = 20; /* default */
    
    struct TagItem *lv_width_tag, *lv_height_tag;
    WORD lv_width, lv_height, viewheight;
    
    WORD ysize, totalitemheight = 0;
    
    struct TagItem *tag, lvtags[] =
    {
    	{GA_Disabled,		FALSE},
    	{GTLV_Top,		0L},
    	{GTLV_MakeVisible,	0L},
    	{GTLV_Labels,		(IPTR)NULL},
    	{GTLV_Selected,		0L},
    	{GTLV_ItemHeight,	0L},
    	{GTLV_CallBack,		(IPTR)NULL},
    	{GTLV_MaxPen,		0L},
    	{GTLV_ReadOnly,		0L},
    	{LAYOUTA_Spacing,	0L},    	
    	{GA_TextAttr,		(IPTR)NULL},
	{TAG_MORE, 	(IPTR)NULL}
    };
    
    EnterFunc(bug("makelistview()\n"));
    
    /* Could use GetTagData(), but this is faster */
    while ((tag = NextTagItem(&taglist)))
    {
    	IPTR tidata = tag->ti_Data;
    	
    	switch (tag->ti_Tag)
    	{
    	case GA_Disabled:	lvtags[0].ti_Data = tidata; break;
    	case GTLV_Top:		lvtags[1].ti_Data = tidata; break;
    	case GTLV_MakeVisible:	lvtags[2].ti_Data = tidata; break;
    	case GTLV_Labels:	lvtags[3].ti_Data = tidata; break;
    	case GTLV_Selected:	lvtags[4].ti_Data = tidata; break;
    	case GTLV_ItemHeight:	lvtags[5].ti_Data = tidata; break;
    	case GTLV_CallBack:	lvtags[6].ti_Data = tidata; break;
    	case GTLV_MaxPen:	lvtags[7].ti_Data = tidata; break;
    	case GTLV_ReadOnly:	lvtags[8].ti_Data = tidata; break;
    	case LAYOUTA_Spacing:	lvtags[9].ti_Data = tidata; break;
    	
    	case GTLV_ShowSelected:	showselgad  = (struct Gadget *)tidata; break;
    	case GTLV_ScrollWidth:	scroller_width = (UWORD)tidata;
    	}
    	
    } /* while (iterate taglist) */

    /* Callback supplied ? */
    if (!lvtags[6].ti_Data)
    	lvtags[6].ti_Tag = TAG_IGNORE; /* Don't pass GTLV_Callback, NULL */

    if (tattr) /* Text Attr supplied ? */
    {
	lvtags[10].ti_Data = (IPTR)tattr; 
	ysize = tattr->ta_YSize;
    }
    else
    {
    	lvtags[10].ti_Tag = TAG_IGNORE; /* Don't pass GA_TextAttr, NULL */
    	
    	ysize = vi->vi_dri->dri_Font->tf_YSize;
    }
    
    /* If not set allready, set ItemHeight */
    if (lvtags[5].ti_Data == 0)
    	lvtags[5].ti_Data = ysize;
    	
    /* If not set allready, set Spacing */
    if (lvtags[9].ti_Data == 0)
    	lvtags[9].ti_Data = LV_DEF_INTERNAL_SPACING;
    	
    D(bug("item ysize: %d\n", ysize));
    
    
    /* Find the dimension specific tags */
    lv_width_tag  = FindTagItem(GA_Width,  stdgadtags);
    lv_height_tag = FindTagItem(GA_Height, stdgadtags);
    
    lv_width  = (WORD)lv_width_tag->ti_Data;
    lv_height = (WORD)lv_height_tag->ti_Data;
    
    	
    /* Adjust the listview width according to scroller width + some spacing */
    lv_width -= (scroller_width + SCROLLER_SPACING);
    
    /* Adjust the listview height according to showsel gadget height */
    if (showselgad)
    {
    	lv_height -= (showselgad->Height + 4);
    	D(bug("Showselected gadget specified"));

    }
    
			/* GTLV_ItemHeight + LAYOUTA_Spacing */
    totalitemheight = lvtags[5].ti_Data + lvtags[9].ti_Data;
    
    /* Adjust listview height so that an exact number of items fits in it */
    viewheight = lv_height - (2 * LV_BORDER_Y);
    lv_height -= (viewheight % totalitemheight);
    
    /* Reinsert the modified dimension attrs into the listview taglist */
    
    D(bug("Dimension passed to listview: w=%d, h=%d\n", lv_width, lv_height));
    lv_width_tag->ti_Data  = (IPTR)lv_width;
    lv_height_tag->ti_Data = (IPTR)lv_height;
    
    
    lvtags[11].ti_Data = (IPTR)stdgadtags;
    
    cl = makelistviewclass(GadToolsBase);
    if (cl)
    {
    	D(bug("Listview class opened\n"));
    	lvgad = (struct Gadget *)NewObjectA(cl, NULL, lvtags);
    	if (lvgad)
        {
    	    struct TagItem scr_stdtags[] =
    	    {
           	{GA_Left, 	0L},
	   	{GA_Top, 	0L},
	    	{GA_Width, 	0L},
	    	{GA_Height, 	0L},
	    	{GA_Previous, (IPTR)NULL},
	    	{GA_ID, 	0L},
	    	{GA_DrawInfo, (IPTR)NULL},
	    	{TAG_END, 	0L}
            };
        
            struct TagItem scr_specialtags[] =
            {
            	/* The listview will initialize the scrollers top, visible & total,
            	** in its GM_LAYOUT method
            	*/
            	{GTSC_Arrows, 	10L},
            	{PGA_Freedom,	LORIENT_VERT},
            	{TAG_DONE,}
            };
        
            D(bug("Listview gadget created: %p\n", lvgad));
    	    /* Create a scroller object to use with the listviev */
    	    scr_stdtags[0].ti_Data = lvgad->LeftEdge + lvgad->Width  - 1 + SCROLLER_SPACING;
    	    scr_stdtags[1].ti_Data = lvgad->TopEdge;
    	    scr_stdtags[2].ti_Data = scroller_width;
    	    scr_stdtags[3].ti_Data = lvgad->Height;
    	    scr_stdtags[4].ti_Data = (IPTR)lvgad;
    	    scr_stdtags[5].ti_Data = lvgad->GadgetID;
    	    scr_stdtags[6].ti_Data = (IPTR)vi->vi_dri;
    	
    	    scrollergad = makescroller(GadToolsBase, scr_stdtags, vi, scr_specialtags);
    	    if (scrollergad)
    	    {
    	    	struct TagItem lvset_tags[] = {{GTA_Listview_Scroller, (IPTR)NULL}, {TAG_DONE, }};
    	    	struct TagItem scrnotify_tags[] =
    	    	{
    	    	    {ICA_TARGET, 	(IPTR)NULL},
    	    	    {ICA_MAP,		(IPTR)NULL},
    	    	    {TAG_DONE, }
    	    	};

                D(bug("Scroller gadget created: %p\n", scrollergad));
                    	    
    	    	/* Tell the listview about the scroller and the showselgad */
    	    	lvset_tags[0].ti_Data = (IPTR)scrollergad;
    	    	SetAttrsA((Object *)lvgad, lvset_tags);
    	    	
    	    	/* Tell the scroller to notify the listview when its PGA_Top attribute changes */
    	    	scrnotify_tags[0].ti_Data = (IPTR)lvgad;
    	    	scrnotify_tags[1].ti_Data = (IPTR)scroller2lv;
    	    	
    	    	SetAttrsA((Object *)scrollergad, scrnotify_tags);
    	    	
    		ReturnPtr ("makelistview", struct Gadget *, lvgad);
    	    }
    	    
    	} /* if (lvgad created) */
    	
    } /* if (listviewclass initialized) */

    ReturnPtr ("makelistview", struct Gadget *, NULL);
}
