/*
    Copyright � 1999, David Le Corfec.
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#include <exec/types.h>
#include <exec/memory.h>

#include <string.h>

#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <clib/alib_protos.h>
#include <graphics/gfxmacros.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/commodities.h>
#include <proto/layers.h>
#include <proto/gadtools.h>
#include <proto/muimaster.h>

#define MUI_OBSOLETE /* for the obsolete menu stuff */

#include "mui.h"
#include "support.h"
#include "classes/window.h"
#include "classes/area.h"
#include "imspec.h"
#include "prefs.h"
#include "dragndrop.h"

#include "muimaster_intern.h"

//#define MYDEBUG 1
#include "debug.h"

extern struct Library *MUIMasterBase;

static const int __version = 1;
static const int __revision = 1;

#define IM(x) ((struct Image*)(x))
#define G(x) ((struct Gadget*)(x))
#define GADGETID(x) (((struct Gadget*)(x))->GadgetID)

/* this is for the cycle list */
struct ObjNode
{
    struct MinNode node;
    Object *obj;
};

struct IDNode  /* For the gadget ids */
{
    struct MinNode node;
    LONG id;
};

struct MUI_ImageSpec_intern;

struct MUI_WindowData
{
    struct MUI_RenderInfo wd_RenderInfo;
    struct MUI_MinMax     wd_MinMax;
    struct IBox    wd_AltDim;       /* zoomed dimensions */
    struct MinList wd_CycleChain;   /* objects activated with tab */
    struct MinList wd_EHList;       /* event handlers */
    struct MinList wd_CCList;       /* control chars */
    struct MinList wd_IDList;       /* gadget ids */
    ULONG          wd_Events;       /* events received */
    ULONG          wd_CrtFlags;     /* window creation flags, see below */
    Object        *wd_ActiveObject; /* the active object */
    Object        *wd_OldActive;    /* active object before window was closed */
    APTR           wd_DefaultObject;
    ULONG          wd_ID;
    STRPTR         wd_Title;
    STRPTR         wd_ScreenTitle;
    LONG           wd_Height;       /* Current dimensions */
    LONG           wd_Width;
    LONG           wd_X;
    LONG           wd_Y;
    LONG           wd_ReqHeight;    /* given by programmer */
    LONG           wd_ReqWidth;
    APTR           wd_RootObject;   /* unique child */
    ULONG          wd_Flags;        /* various status flags */
    struct MUI_ImageSpec_intern *wd_Background;
    ULONG          wd_DisabledKeys;
    BOOL           wd_NoMenus;     /* MUIA_Window_NoMenus */
    
    Object        *wd_DragObject; /* the object which is being dragged */
    struct Window *wd_DropWindow; /* the destination window, for faster access */
    Object        *wd_DropObject; /* the destination object */
    struct DragNDrop     *wd_dnd;
    struct MUI_DragImage *wd_DragImage;

    Object        *wd_Menustrip; /* The menustrip object which is actually is used (eighter apps or windows or NULL) */
    Object *       wd_ChildMenustrip; /* If window has an own Menustrip */
    struct Menu   *wd_Menu; /* the intuition menustrip */

    Object *wd_VertProp;
    Object *wd_UpButton;
    Object *wd_DownButton;

    Object *wd_HorizProp;
    Object *wd_LeftButton;
    Object *wd_RightButton;
    Object *wd_RefWindow;
};

#ifndef WFLG_SIZEGADGET

#define WFLG_CLOSEGADGET (1<<0) /* has close gadget */
#define WFLG_SIZEGADGET  (1<<1) /* has size gadget */
#define WFLG_BACKDROP    (1<<2) /* is backdrop window */
#define WFLG_BORDERLESS  (1<<3) /* has no borders */
#define WFLG_DEPTHGADGET (1<<4) /* has depth gadget */
#define WFLG_DRAGBAR     (1<<5) /* is draggable */
#define WFLG_SIZEBRIGHT  (1<<6) /* size gadget is in right border */

#endif

/* wd_Flags */
#define MUIWF_OPENED          (1<<0) /* window currently opened */
#define MUIWF_ICONIFIED       (1<<1) /* window currently iconified */
#define MUIWF_ACTIVE          (1<<2) /* window currently active */
#define MUIWF_RESIZING        (1<<4) /* window currently resizing, for simple refresh */
#define MUIWF_DONTACTIVATE    (1<<7) /* do not activate the window when opening */
#define MUIWF_USERIGHTSCROLLER (1<<8) /* window should have a right scroller */
#define MUIWF_USEBOTTOMSCROLLER (1<<9) /* windiw should have a bottom scroller */
#define MUIWF_ERASEAREA       (1<<10) /* Erase area after a window resize */
#define MUIWF_ISAPPWINDOW     (1<<11) /* Is an app window (user can drop icons on it) */
#define MUIWF_ISSUBWINDOW     (1<<12) /* Dont get automatically disposed with app */

struct __dummyXFC3__
{
	struct MUI_NotifyData mnd;
	struct MUI_WindowData mwd;
};

#define muiWindowData(obj)   (&(((struct __dummyXFC3__ *)(obj))->mwd))

static ULONG DoHalfshineGun(ULONG a, ULONG b)
{
    ULONG val = ((((a)>>24) + 3 * ((b)>>24)) / 4);
    val = val + (val<<8) + (val<<16) + (val<<24);
    return val;
}

static ULONG DoHalfshadowGun(ULONG a, ULONG b)
{
    ULONG val = ((((a)>>24) + 5 * ((b)>>24)) / 6);
    val = val + (val<<8) + (val<<16) + (val<<24);
    return val;
}

static Object *CreateSysimage(struct DrawInfo *dri, ULONG which)
{
    return NewObject(NULL, "sysiclass",
		     SYSIA_DrawInfo, (IPTR)dri,
		     SYSIA_Which, which,
		     TAG_DONE);
}

static BOOL SetupRenderInfo(Object *obj, struct MUI_WindowData *data, struct MUI_RenderInfo *mri)
{
    ULONG rgbtable[3 * 3];
    Object *temp_obj;
    ULONG val;
    int i;

    if (!(mri->mri_Screen = LockPubScreen(NULL))) return FALSE;
    if (!(mri->mri_DrawInfo = GetScreenDrawInfo(mri->mri_Screen)))
    {
	UnlockPubScreen(NULL,mri->mri_Screen);
	return FALSE;
    }

    mri->mri_Colormap     = mri->mri_Screen->ViewPort.ColorMap;
    mri->mri_ScreenWidth  = mri->mri_Screen->Width;
    mri->mri_ScreenHeight = mri->mri_Screen->Height;

    if (mri->mri_ScreenWidth / mri->mri_ScreenHeight < 2)
    {
	mri->mri_Flags |= MUIMRI_THINFRAMES;
    }

    if (GetBitMapAttr(mri->mri_Screen->RastPort.BitMap, BMA_DEPTH) >= 15)
    {
	mri->mri_Flags |= MUIMRI_TRUECOLOR;
    }

    mri->mri_PensStorage[MPEN_SHINE] = mri->mri_DrawInfo->dri_Pens[SHINEPEN];
    mri->mri_PensStorage[MPEN_BACKGROUND] = mri->mri_DrawInfo->dri_Pens[BACKGROUNDPEN];
    mri->mri_PensStorage[MPEN_SHADOW] = mri->mri_DrawInfo->dri_Pens[SHADOWPEN];
    mri->mri_PensStorage[MPEN_TEXT] = mri->mri_DrawInfo->dri_Pens[TEXTPEN];
    mri->mri_PensStorage[MPEN_FILL] = mri->mri_DrawInfo->dri_Pens[FILLPEN];

    GetRGB32(mri->mri_Colormap, mri->mri_DrawInfo->dri_Pens[SHINEPEN], 1, rgbtable);
    GetRGB32(mri->mri_Colormap, mri->mri_DrawInfo->dri_Pens[BACKGROUNDPEN], 1, rgbtable+3);
    GetRGB32(mri->mri_Colormap, mri->mri_DrawInfo->dri_Pens[SHADOWPEN], 1, rgbtable+6);

    mri->mri_PensStorage[MPEN_HALFSHINE] = ObtainBestPenA
    (
        mri->mri_Colormap,
        DoHalfshineGun(rgbtable[0], rgbtable[3]),
        DoHalfshineGun(rgbtable[1], rgbtable[4]),
        DoHalfshineGun(rgbtable[2], rgbtable[5]), NULL
    );

    mri->mri_PensStorage[MPEN_HALFSHADOW] = ObtainBestPenA
    (
        mri->mri_Colormap,
        DoHalfshadowGun(rgbtable[6], rgbtable[3]),
        DoHalfshadowGun(rgbtable[7], rgbtable[4]),
        DoHalfshadowGun(rgbtable[8], rgbtable[5]), NULL
    );

/* I'm really not sure that MUI does this for MPEN_MARK, but it seems mostly acceptable -dlc */
    mri->mri_PensStorage[MPEN_MARK] = ObtainBestPenA
    (
        mri->mri_Colormap, 0xf4f4f4f4, 0xb5b5b5b5, 0x8b8b8b8b, NULL
    );

    mri->mri_Pens = mri->mri_PensStorage;

    for (i = 0; i < -MUIV_Font_NegCount; i++)
    {
	mri->mri_Fonts[i] = NULL;
    }

    mri->mri_LeftImage  = CreateSysimage(mri->mri_DrawInfo, LEFTIMAGE);
    mri->mri_RightImage = CreateSysimage(mri->mri_DrawInfo, RIGHTIMAGE);
    mri->mri_UpImage    = CreateSysimage(mri->mri_DrawInfo, UPIMAGE);
    mri->mri_DownImage  = CreateSysimage(mri->mri_DrawInfo, DOWNIMAGE);
    mri->mri_SizeImage  = CreateSysimage(mri->mri_DrawInfo, SIZEIMAGE);

    if (data->wd_CrtFlags & WFLG_BORDERLESS)
    {
    	/* Infact borderless windows could also have borders (if they have a window title e.g. but
    	   since they look ugly anywhy we ignore it for now */
	mri->mri_BorderLeft = 0;
	mri->mri_BorderRight = 0;
	mri->mri_BorderTop = 0;
	mri->mri_BorderBottom = 0;
    }
    else
    {
	mri->mri_BorderLeft = mri->mri_Screen->WBorLeft;
	mri->mri_BorderTop = mri->mri_Screen->WBorTop + mri->mri_Screen->Font->ta_YSize+ 1;
	temp_obj = NewObject
        (
            NULL, "sysiclass",
    	    SYSIA_DrawInfo, (IPTR)mri->mri_DrawInfo,
    	    SYSIA_Which,    SIZEIMAGE,
    	    TAG_DONE
        );
	if (temp_obj)
	{
	    GetAttr(IA_Height,temp_obj,&val);
    	    DisposeObject(temp_obj);
    	    mri->mri_BorderBottom = val;
        }
	else
	    mri->mri_BorderBottom = mri->mri_Screen->WBorBottom;
    }

    return TRUE;
}

static void CleanupRenderInfo(struct MUI_RenderInfo *mri)
{
    int i;

    if (mri->mri_LeftImage) {DisposeObject(mri->mri_LeftImage);mri->mri_LeftImage=NULL;};
    if (mri->mri_RightImage){DisposeObject(mri->mri_RightImage);mri->mri_RightImage=NULL;};
    if (mri->mri_UpImage) {DisposeObject(mri->mri_UpImage);mri->mri_UpImage=NULL;};
    if (mri->mri_DownImage) {DisposeObject(mri->mri_DownImage);mri->mri_DownImage=NULL;};
    if (mri->mri_SizeImage) {DisposeObject(mri->mri_SizeImage);mri->mri_SizeImage=NULL;};

    for (i = 0; i < -MUIV_Font_NegCount; i++)
    {
	if (mri->mri_Fonts[i])
	{
	    CloseFont(mri->mri_Fonts[i]);
	    mri->mri_Fonts[i] = NULL;
	}
    }
    ReleasePen(mri->mri_Colormap, mri->mri_PensStorage[MPEN_MARK]);
    ReleasePen(mri->mri_Colormap, mri->mri_PensStorage[MPEN_HALFSHADOW]);
    ReleasePen(mri->mri_Colormap, mri->mri_PensStorage[MPEN_HALFSHINE]);
    FreeScreenDrawInfo(mri->mri_Screen, mri->mri_DrawInfo);
    mri->mri_DrawInfo = NULL;

    UnlockPubScreen(NULL, mri->mri_Screen);
    mri->mri_Screen = NULL;
}

static void ShowRenderInfo(struct MUI_RenderInfo *mri)
{
    if (mri->mri_BufferBM)
    {
	mri->mri_RastPort = &mri->mri_BufferRP;
    }
    else
    {
	mri->mri_RastPort = mri->mri_Window->RPort;
    }
}

static void HideRenderInfo(struct MUI_RenderInfo *mri)
{
    mri->mri_RastPort = NULL;
}



static ULONG _zune_window_get_default_events (void)
{
    return IDCMP_NEWSIZE      | IDCMP_REFRESHWINDOW
         | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_MENUPICK
         | IDCMP_CLOSEWINDOW  | IDCMP_RAWKEY
         | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW;
}

static void _zune_window_change_events (struct MUI_WindowData *data, ULONG new_events)
{
    struct MinNode *mn;
    struct MUI_EventHandlerNode *ehn;
    ULONG old_events = data->wd_Events;

    for (mn = data->wd_EHList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
    {
        ehn = (struct MUI_EventHandlerNode *)mn;
        new_events |= ehn->ehn_Events;
    }

    /* sba: kill the IDCMP_VANILLAKEY flag. MUI doesn't do this but programs
    ** which use this will behave different if they request for this flag
    ** (also on MUI)
    */
    new_events &= ~IDCMP_VANILLAKEY;
    
    data->wd_Events = new_events;
    if ((old_events != new_events) && (data->wd_Flags & MUIWF_OPENED))
    {
        ModifyIDCMP(data->wd_RenderInfo.mri_Window, new_events);
    }
}

static BOOL DisplayWindow(Object *obj, struct MUI_WindowData *data)
{
    struct Window *win;
    ULONG flags = data->wd_CrtFlags;
    struct MUI_RenderInfo *mri = &data->wd_RenderInfo;
    struct IBox altdims;
    ULONG backfill;

    struct Menu *menu = NULL;
    struct NewMenu *newmenu = NULL;
    APTR visinfo = NULL;

    Object *firstgad = NULL;
    Object *prevgad = NULL;
    LONG id;

    if (!(data->wd_Flags & MUIWF_DONTACTIVATE))
        flags |= WFLG_ACTIVATE;

    if 
    (
           data->wd_MinMax.MinHeight == data->wd_MinMax.MaxHeight
	&& data->wd_MinMax.MinWidth  == data->wd_MinMax.MaxWidth
    )
	flags &= ~WFLG_SIZEGADGET;

    if (!(flags & WFLG_SIZEBRIGHT))
        flags |= WFLG_SIZEBBOTTOM;

    if (!data->wd_RefWindow)
    {
	/* The following calculations are not very correct, the size and dragbar
	** are ignored also the current overscan view */
	if (data->wd_X == MUIV_Window_LeftEdge_Centered)
	{
	    data->wd_X = (data->wd_RenderInfo.mri_Screen->Width - data->wd_Width)/2;
	}
	else if (data->wd_X == MUIV_Window_LeftEdge_Moused)
	{
	}
    }
    else
    {
	ULONG w = 0, x = 0;

	get(data->wd_RefWindow, MUIA_Window_Width, &w);
	get(data->wd_RefWindow, MUIA_Window_LeftEdge, &x);

	if (data->wd_X == MUIV_Window_LeftEdge_Centered)
	{
	    data->wd_X = x + (w - data->wd_Width)/2;
	}
	else
	{
	    data->wd_X += x; 
	}
    }


    if (!data->wd_RefWindow)
    {
	if (data->wd_Y == MUIV_Window_TopEdge_Centered)
	{
	    data->wd_Y = (data->wd_RenderInfo.mri_Screen->Height - data->wd_Height)/2;
	}
	else if (data->wd_Y == MUIV_Window_TopEdge_Moused)
	{
	}
	else if (data->wd_Y <= MUIV_Window_TopEdge_Delta(0))
	{
	    data->wd_Y = data->wd_RenderInfo.mri_Screen->BarHeight + 1
		+ MUIV_Window_TopEdge_Delta(0) - data->wd_Y;
	}
    }
    else
    {
	ULONG h = 0, y = 0;

	get(data->wd_RefWindow, MUIA_Window_Height, &h);
	get(data->wd_RefWindow, MUIA_Window_TopEdge, &y);

	if (data->wd_Y == MUIV_Window_TopEdge_Centered)
	{
/*  	    D(bug("y=%ld, h=%ld, wdh=%ld\n", y, h, data->wd_Height)); */
	    data->wd_Y = y + (h - data->wd_Height)/2;
	}
	else if (data->wd_Y <= MUIV_Window_TopEdge_Delta(0))
	{
	    /* ??? surely incorrect implementation */
	    data->wd_Y = y + 1 + MUIV_Window_TopEdge_Delta(0) - data->wd_Y;
	}
	else
	{
	    data->wd_Y += y;
	}
    }

    if ((visinfo = GetVisualInfoA(data->wd_RenderInfo.mri_Screen,NULL)))
    {
	if (data->wd_Menustrip)
	{
	    get(data->wd_Menustrip,MUIA_Menuitem_NewMenu,&newmenu);
	    if (newmenu)
	    {
		if ((menu = CreateMenusA(newmenu,NULL)))
		{
		    struct TagItem tags[] =
                    {
                        {GTMN_NewLookMenus,TRUE},
                        {TAG_DONE, NULL}
                    };
                    LayoutMenusA(menu, visinfo, tags);
                }
	    }
	}
	FreeVisualInfo(visinfo);
    }

    /* Create the right border scrollers now if requested */
    if (data->wd_Flags & MUIWF_USERIGHTSCROLLER)
    {
    	int voffset;

	voffset = IM(mri->mri_DownImage)->Width / 4;

	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	firstgad = prevgad = data->wd_VertProp = NewObject
        (
            NULL, "propgclass",
            
            GA_RelRight,    1 - (IM(mri->mri_UpImage)->Width - voffset),
            GA_Top,         mri->mri_BorderTop + 2,
            GA_Width,       IM(mri->mri_UpImage)->Width - voffset * 2,
            GA_RelHeight,   - (mri->mri_BorderTop + 2) 
                            - IM(mri->mri_UpImage)->Height 
                            - IM(mri->mri_DownImage)->Height 
                            - IM(mri->mri_SizeImage)->Height - 2,
            GA_RightBorder, TRUE,
            GA_ID,          id,
            PGA_Borderless, TRUE,
            PGA_NewLook,    TRUE,
            PGA_Freedom,    FREEVERT,
            PGA_Top,        0,
            PGA_Total,      2,
            PGA_Visible,    1,
            ICA_TARGET,     ICTARGET_IDCMP,
            TAG_DONE
        );

	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	prevgad = data->wd_UpButton = NewObject
        (
            NULL, "buttongclass",
            
            GA_Image,       (IPTR)mri->mri_UpImage,
            GA_RelRight,    1 - IM(mri->mri_UpImage)->Width,
            GA_RelBottom,   1 - IM(mri->mri_UpImage)->Height 
                              - IM(mri->mri_DownImage)->Height 
                              - IM(mri->mri_SizeImage)->Height,
            GA_RightBorder, TRUE,
            GA_Previous,    (IPTR)prevgad,
            GA_ID,          id,
            ICA_TARGET,     ICTARGET_IDCMP,
            TAG_DONE
        );

	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	prevgad = data->wd_DownButton = NewObject
        (
            NULL, "buttongclass",
            
            GA_Image,       (IPTR)mri->mri_DownImage,
            GA_RelRight,    1 - IM(mri->mri_DownImage)->Width,
            GA_RelBottom,   1 - IM(mri->mri_DownImage)->Height 
                              - IM(mri->mri_SizeImage)->Height,
            GA_RightBorder, TRUE,
            GA_Previous,    (IPTR)prevgad,
            GA_ID,          id,
            ICA_TARGET,     ICTARGET_IDCMP,
            TAG_DONE
        );
    }

    /* Create the bottom border scrollers now if requested */
    if (data->wd_Flags & MUIWF_USEBOTTOMSCROLLER)
    {
    	int hoffset;

	hoffset = IM(mri->mri_RightImage)->Height / 4;

	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	prevgad = data->wd_HorizProp = NewObject
        (
            NULL, "propgclass",
            
            GA_RelBottom,                       1 - (IM(mri->mri_LeftImage)->Height - hoffset),
            GA_Left,                            mri->mri_BorderLeft,
            GA_Height,                          IM(mri->mri_LeftImage)->Height 
                                                - hoffset * 2,
            GA_RelWidth,                        - (mri->mri_BorderLeft) 
                                                - IM(mri->mri_LeftImage)->Width 
                                                - IM(mri->mri_RightImage)->Width 
                                                - IM(mri->mri_SizeImage)->Width 
                                                - 2,
            GA_BottomBorder,                    TRUE,
            GA_ID,                              id,
            prevgad ? GA_Previous : TAG_IGNORE, (IPTR)prevgad,
            PGA_Borderless,                     TRUE,
            PGA_NewLook,                        TRUE,
            PGA_Freedom,                        FREEHORIZ,
            PGA_Top,                            0,
            PGA_Total,                          2,
            PGA_Visible,                        1,
            ICA_TARGET,                         ICTARGET_IDCMP,
            TAG_DONE
        );

	if (!firstgad) firstgad = prevgad;

	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	prevgad = data->wd_LeftButton = NewObject
        (
            NULL, "buttongclass",
            
            GA_Image,        (IPTR)mri->mri_LeftImage,
            GA_RelRight,     1 - IM(mri->mri_LeftImage)->Width 
                               - IM(mri->mri_RightImage)->Width 
                               - IM(mri->mri_SizeImage)->Width,
            GA_RelBottom,    1 - IM(mri->mri_LeftImage)->Height,
            GA_BottomBorder, TRUE,
            GA_Previous,     (IPTR)prevgad,
            GA_ID,           id,
            ICA_TARGET,      ICTARGET_IDCMP,
            TAG_DONE
        );
        
	id = DoMethod(obj, MUIM_Window_AllocGadgetID);
	prevgad = data->wd_RightButton = NewObject
        (
            NULL, "buttongclass",
            
            GA_Image,        (IPTR)mri->mri_RightImage,
            GA_RelRight,     1 - IM(mri->mri_RightImage)->Width 
                               - IM(mri->mri_SizeImage)->Width,
            GA_RelBottom,    1 - IM(mri->mri_RightImage)->Height,
            GA_BottomBorder, TRUE,
            GA_Previous,     (IPTR)prevgad,
            GA_ID,           id,
            ICA_TARGET,      ICTARGET_IDCMP,
            TAG_DONE);
    }

/* Calculate alternate (zoomed) dimensions.
 */
    if (data->wd_AltDim.Top == MUIV_Window_AltTopEdge_NoChange)
	data->wd_AltDim.Top = ~0;
    else if (data->wd_AltDim.Top == MUIV_Window_AltTopEdge_Centered)
	data->wd_AltDim.Top = (data->wd_RenderInfo.mri_Screen->Height - data->wd_Height)/2;
    else if (data->wd_AltDim.Top == MUIV_Window_AltTopEdge_Moused)
	/* ? */ data->wd_AltDim.Top = ~0;

    if (data->wd_AltDim.Left == MUIV_Window_AltLeftEdge_NoChange)
	data->wd_AltDim.Left = ~0;
    else if (data->wd_AltDim.Left == MUIV_Window_AltLeftEdge_Centered)
	data->wd_AltDim.Left = (data->wd_RenderInfo.mri_Screen->Width - data->wd_Width)/2;
    else if (data->wd_AltDim.Left == MUIV_Window_AltLeftEdge_Moused)
	/* ? */ data->wd_AltDim.Left = ~0;

    if 
    (
        _between
        (
            MUIV_Window_AltWidth_MinMax(100),
            data->wd_AltDim.Width,
            MUIV_Window_AltWidth_MinMax(0)
        )
    )
    {
	data->wd_AltDim.Width = data->wd_MinMax.MinWidth
	    - data->wd_AltDim.Width
	    * (data->wd_MinMax.MaxWidth - data->wd_MinMax.MinWidth);
    }
    else if
    (
        _between
        (
            MUIV_Window_AltWidth_Screen(100),
            data->wd_AltDim.Width,
            MUIV_Window_AltWidth_Screen(0)
        )
    )
    {
	data->wd_AltDim.Width = data->wd_RenderInfo.mri_ScreenWidth
	    * (- (data->wd_AltDim.Width + 200)) / 100;
    }
    else if
    (
        _between
        (
            MUIV_Window_AltWidth_Visible(100),
            data->wd_AltDim.Width,
            MUIV_Window_AltWidth_Visible(0)
        )
    )
    {
	data->wd_AltDim.Width = data->wd_RenderInfo.mri_ScreenWidth
	    * (- (data->wd_AltDim.Width + 100)) / 100;
    }

    if
    (
        _between
        (
            MUIV_Window_AltHeight_MinMax(100),
            data->wd_AltDim.Height,
            MUIV_Window_AltHeight_MinMax(0)
        )
    )
    {
	data->wd_AltDim.Height = data->wd_MinMax.MinHeight
	    - data->wd_AltDim.Height
	    * (data->wd_MinMax.MaxHeight - data->wd_MinMax.MinHeight);
    }
    else if
    (
        _between
        (
            MUIV_Window_AltHeight_Screen(100),
            data->wd_AltDim.Height,
            MUIV_Window_AltHeight_Screen(0)
        )
    )
    {
	data->wd_AltDim.Height = data->wd_RenderInfo.mri_ScreenHeight
	    * (- (data->wd_AltDim.Height + 200)) / 100;
    }
    else if
    (
        _between
        (
            MUIV_Window_AltHeight_Visible(100),
            data->wd_AltDim.Height,
            MUIV_Window_AltHeight_Visible(0)
        )
    )
    {
	data->wd_AltDim.Height = data->wd_RenderInfo.mri_ScreenHeight
	    * (- (data->wd_AltDim.Height + 100)) / 100;
    }

    data->wd_AltDim.Width = CLAMP
    (
        data->wd_AltDim.Width, data->wd_MinMax.MinWidth, 
        data->wd_MinMax.MaxWidth
    );
    data->wd_AltDim.Height = CLAMP
    (
        data->wd_AltDim.Height, data->wd_MinMax.MinHeight,
        data->wd_MinMax.MaxHeight
    );

    altdims = data->wd_AltDim;
    /* hack to account for border size, as we only know the innersize and must give
     * the total size.
     */
    altdims.Width += data->wd_RenderInfo.mri_Screen->WBorLeft + data->wd_RenderInfo.mri_Screen->WBorRight;
    altdims.Height += data->wd_RenderInfo.mri_Screen->WBorTop + data->wd_RenderInfo.mri_Screen->WBorBottom + data->wd_RenderInfo.mri_DrawInfo->dri_Font->tf_YSize + 11;
    
    if (muiGlobalInfo(obj)->mgi_Prefs->window_redraw == WINDOW_REDRAW_WITHOUT_CLEAR)
	backfill = WA_BackFill;
    else
	backfill = TAG_IGNORE;

    win = OpenWindowTags
    (
        NULL,
        
        WA_Left,                (IPTR) data->wd_X,
        WA_Top,                 (IPTR) data->wd_Y,
        WA_Flags,               (IPTR) flags,
        data->wd_Title ?
            WA_Title   : 
            TAG_IGNORE,         (IPTR) data->wd_Title,
        data->wd_ScreenTitle ? 
            WA_ScreenTitle   : 
            TAG_IGNORE,         (IPTR) data->wd_ScreenTitle,
        WA_CustomScreen,        (IPTR) data->wd_RenderInfo.mri_Screen,
        WA_InnerWidth,          (IPTR) data->wd_Width,
        WA_InnerHeight,         (IPTR) data->wd_Height,
        WA_AutoAdjust,          (IPTR) TRUE,
        WA_NewLookMenus,        (IPTR) TRUE,
        data->wd_NoMenus ? 
            WA_RMBTrap   : 
            TAG_IGNORE,         (IPTR) TRUE,
        WA_Gadgets,             (IPTR) data->wd_VertProp,
        WA_Zoom,                (IPTR) &altdims,
        backfill,               (IPTR) LAYERS_NOBACKFILL,
        TAG_DONE
    );

    if (win)
    {
        int hborders = win->BorderLeft + win->BorderRight;
        int vborders = win->BorderTop  + win->BorderBottom;

        /* recalc window size (which will hopefully equal our requested size) */
        data->wd_Width  = win->GZZWidth;
        data->wd_Height = win->GZZHeight;

        /* set window limits according to window contents */
        WindowLimits
        (
            win, data->wd_MinMax.MinWidth  + hborders,
            data->wd_MinMax.MinHeight + vborders,
            data->wd_MinMax.MaxWidth  + hborders,
            data->wd_MinMax.MaxHeight + vborders
        );
        
        win->UserData = (char*)data->wd_RenderInfo.mri_WindowObject;
        win->UserPort = muiGlobalInfo(obj)->mgi_WindowsPort; /* Same port for all windows */
        ModifyIDCMP(win, data->wd_Events);

        data->wd_RenderInfo.mri_Window = win;
        data->wd_RenderInfo.mri_VertProp = data->wd_VertProp;
        data->wd_RenderInfo.mri_HorizProp = data->wd_HorizProp;
	if (menu)
	{
	    data->wd_Menu = menu;
	    SetMenuStrip(win,menu);
	}

        return TRUE;
    }

    if (menu) FreeMenus(menu);

    return FALSE;
}

static void UndisplayWindow(Object *obj, struct MUI_WindowData *data)
{
    struct Window *win = data->wd_RenderInfo.mri_Window;

    if (win != NULL)
    {
        data->wd_RenderInfo.mri_Window = NULL;
        data->wd_RenderInfo.mri_VertProp = NULL;
        data->wd_RenderInfo.mri_HorizProp = NULL;

        /* store position and size */
        data->wd_X      = win->LeftEdge;
        data->wd_Y      = win->TopEdge;
        data->wd_Width  = win->GZZWidth;
        data->wd_Height = win->GZZHeight;

        ClearMenuStrip(win);
        if (data->wd_Menu)
        {
            FreeMenus(data->wd_Menu);
	    data->wd_Menu = NULL;
        }

        if (win->UserPort)
        {
            struct IntuiMessage *msg, *succ;

            /* remove all messages pending for this window */
            Forbid();
            for
            (
                msg  = (struct IntuiMessage *)win->UserPort->mp_MsgList.lh_Head;
                (succ = (struct IntuiMessage *)msg->ExecMessage.mn_Node.ln_Succ);
                msg  = succ
            )
            {
                if (msg->IDCMPWindow == win)
                {
                    Remove((struct Node *)msg);
                    ReplyMsg((struct Message *)msg);
                }
            }
            win->UserPort = NULL;
            ModifyIDCMP(win, 0);
            Permit();
        }

/*  	D(bug("before CloseWindow\n")); */
        CloseWindow(win);
/*  	D(bug("after CloseWindow\n")); */

#define DISPOSEGADGET(x) \
	if (x)\
	{\
	    DoMethod(obj, MUIM_Window_FreeGadgetID, ((struct Gadget*)x)->GadgetID);\
	    DisposeObject(x);\
	    x = NULL;\
	}

	DISPOSEGADGET(data->wd_VertProp);
	DISPOSEGADGET(data->wd_UpButton);
	DISPOSEGADGET(data->wd_DownButton);
	DISPOSEGADGET(data->wd_HorizProp);
	DISPOSEGADGET(data->wd_LeftButton);
	DISPOSEGADGET(data->wd_RightButton);
    }
}


/* return FALSE only if no resize (dx=dy=0) occured */
static BOOL
_zune_window_resize (struct MUI_WindowData *data)
{
    struct Window *win = data->wd_RenderInfo.mri_Window;
    int hborders = win->BorderLeft + win->BorderRight;
    int vborders = win->BorderTop  + win->BorderBottom;
    WORD dx = data->wd_Width  - win->Width + hborders;
    WORD dy = data->wd_Height - win->Height + vborders;

/*      D(bug("_zune_window_resize : dx=%d, dy=%d\n", dx, dy)); */
    SizeWindow(win, dx, dy);

    /* The following WindowLimits() call doesn't really work because SizeWindow() is async */
    WindowLimits
    (
        win, data->wd_MinMax.MinWidth + hborders,
        data->wd_MinMax.MinHeight + vborders,
        data->wd_MinMax.MaxWidth + hborders,
        data->wd_MinMax.MaxHeight + vborders
    );

    return (dx || dy);
}

/**************/

static BOOL ContextMenuUnderPointer(struct MUI_WindowData *data, Object *obj, LONG x, LONG y)
{
    Object                *cstate;
    Object                *child;
    struct MinList        *ChildList;

    if (!(x >= _left(obj) && x <= _right(obj) 
	  && y >= _top(obj)  && y <= _bottom(obj))) 
    {
        return FALSE;
    }

    if (get(obj, MUIA_Group_ChildList, (ULONG *)&(ChildList)))
    {
        cstate = (Object *)ChildList->mlh_Head;
        while ((child = NextObject(&cstate)))
        {
	    if ((x >= _left(child) && x <= _right(child) 
		 &&
		 y >= _top(child)  && y <= _bottom(child))
		&&
		(ContextMenuUnderPointer(data,child,x,y)))
		return TRUE;
	}
    }

    if (!(muiAreaData(obj)->mad_Flags & MADF_CANDRAW))
	return FALSE;
    if (!(muiAreaData(obj)->mad_ContextMenu))
	return FALSE;

    return TRUE;
}

/**************/

static ULONG window_Open(struct IClass *cl, Object *obj);
static ULONG window_Close(struct IClass *cl, Object *obj);
static void HandleInputEvent(Object *win, struct MUI_WindowData *data,
			     struct IntuiMessage *event);

void HandleDragging (Object *oWin, struct MUI_WindowData *data,
		     struct IntuiMessage *imsg)
{
    struct Window *iWin;
    int finish_drag = 0;

    iWin = imsg->IDCMPWindow;

    if (imsg->Class == IDCMP_MOUSEMOVE)
    {
	struct Layer *layer;
	layer = WhichLayer(&iWin->WScreen->LayerInfo, iWin->LeftEdge + imsg->MouseX, iWin->TopEdge + imsg->MouseY);

	if (data->wd_DropObject)
	{
	    struct Window *wnd;
	    WORD mousex = imsg->MouseX + iWin->LeftEdge - data->wd_DropWindow->LeftEdge;
	    WORD mousey = imsg->MouseY + iWin->TopEdge  - data->wd_DropWindow->TopEdge;

	    wnd = _window(data->wd_DropObject);
	    if
                (
		    mousex < _left(data->wd_DropObject) 
                    || mousex > _right(data->wd_DropObject) 
                    || mousey < _top(data->wd_DropObject) 
                    || mousey > _bottom(data->wd_DropObject) 
                    || layer != wnd->WLayer
		    )
	    {
		/* We have left the object */
		UndrawDragNDrop(data->wd_dnd);
		DoMethod(data->wd_DropObject, MUIM_DragFinish,(IPTR)data->wd_DragObject);
		data->wd_DropObject = NULL;
	    }
	} /* if (data->wd_DropObject) */

	if (!data->wd_DropObject)
	{
	    Object *dest_wnd = NULL;

	    /* Find out if app has an openend window at this position */
	    if (layer)
	    {
		Object                *cstate;
		Object                *child;
		struct MinList        *ChildList;

		get(_app(oWin), MUIA_Application_WindowList, (ULONG *)&(ChildList));
		cstate = (Object *)ChildList->mlh_Head;
		while ((child = NextObject(&cstate)))
		{
		    struct Window *wnd;
		    get(child, MUIA_Window_Window,(ULONG*)&wnd);
		    if (!wnd) continue;

		    if (wnd->WLayer == layer)
		    {
			data->wd_DropWindow = wnd;
			dest_wnd = child;
			break;
		    }
		}
	    } /* if (layer) */

	    if (dest_wnd)
	    {
		Object *root;
		get(dest_wnd, MUIA_Window_RootObject, &root);

		if (root)
		{
		    if
                        (
                            (
                                data->wd_DropObject = (Object*) DoMethod
                                (
                                    root, MUIM_DragQueryExtended, 
                                    (IPTR) data->wd_DragObject,
                                    imsg->MouseX + iWin->LeftEdge - data->wd_DropWindow->LeftEdge,
                                    imsg->MouseY + iWin->TopEdge - data->wd_DropWindow->TopEdge
				    )
				)
			    )
		    {
			UndrawDragNDrop(data->wd_dnd);
			DoMethod(data->wd_DropObject, MUIM_DragBegin,(IPTR)data->wd_DragObject);
		    }
		}
	    } /* if (dest_wnd) */
	} /* if (!data->wd_DropObject) */

	if (data->wd_DropObject)
	{
	    LONG update = 0;
	    LONG i;
	    for (i=0;i<2;i++)
	    {
		LONG res = DoMethod
                    (
                        data->wd_DropObject, MUIM_DragReport, 
                        (IPTR) data->wd_DragObject,
                        imsg->MouseX + iWin->LeftEdge - data->wd_DropWindow->LeftEdge,
                        imsg->MouseY + iWin->TopEdge - data->wd_DropWindow->TopEdge,update
			);
		switch (res)
		{
		    case MUIV_DragReport_Abort:
			UndrawDragNDrop(data->wd_dnd);
			DoMethod(data->wd_DropObject, MUIM_DragFinish,(IPTR)data->wd_DragObject);
			data->wd_DropObject = NULL;
			i = 1;
			break;

		    case MUIV_DragReport_Continue: break;
		    case MUIV_DragReport_Lock: break; /* NYI */
		    case MUIV_DragReport_Refresh:
			UndrawDragNDrop(data->wd_dnd);
			update = 1;
			break;
		}
	    }
	} /* if (data->wd_DropObject) */
	DrawDragNDrop(data->wd_dnd, imsg->MouseX + iWin->LeftEdge , imsg->MouseY + iWin->TopEdge);
    } /* if (imsg->Class == IDCMP_MOUSEMOVE) */

    if (imsg->Class == IDCMP_MOUSEBUTTONS)
    {
	if ((imsg->Code == MENUDOWN)  || (imsg->Code == SELECTUP))
	{
	    if (imsg->Code == SELECTUP && data->wd_DropObject)
	    {
		UndrawDragNDrop(data->wd_dnd);
		DoMethod(data->wd_DropObject, MUIM_DragFinish, (IPTR)data->wd_DragObject);
		DoMethod
                    (
                        data->wd_DropObject, MUIM_DragDrop, (IPTR)data->wd_DragObject,
                        imsg->MouseX + iWin->LeftEdge - data->wd_DropWindow->LeftEdge,
                        imsg->MouseY + iWin->TopEdge - data->wd_DropWindow->TopEdge
			);
		data->wd_DropObject = NULL;
	    }
	    finish_drag = 1;
	}
    }

    if (imsg->Class == IDCMP_CLOSEWINDOW)
	finish_drag = 1;

    if (finish_drag)
    {
	UndrawDragNDrop(data->wd_dnd);
	if (data->wd_DropObject)
	{
	    DoMethod(data->wd_DropObject, MUIM_DragFinish,(IPTR)data->wd_DragObject);
	    data->wd_DropObject = NULL;
	}
	DeleteDragNDrop(data->wd_dnd);
	DoMethod(data->wd_DragObject,MUIM_DeleteDragImage, (IPTR)data->wd_DragImage);
	muiAreaData(data->wd_DragObject)->mad_Flags &= ~MADF_DRAGGING;
	data->wd_DragImage = NULL;
	data->wd_DragObject = NULL;
	data->wd_DropWindow = NULL;
	data->wd_dnd = NULL;
	/* stop listening to IDCMP_MOUSEMOVE */
	_zune_window_change_events(data, _zune_window_get_default_events());
    }
}

/* Reply to imsg if handled */
BOOL HandleWindowEvent (Object *oWin, struct MUI_WindowData *data,
			struct IntuiMessage *imsg)
{
    struct Window *iWin;
    BOOL is_handled = TRUE;

    iWin = imsg->IDCMPWindow;
    switch (imsg->Class)
    {
    	case IDCMP_MOUSEMOVE:
	    if 
            (
                ContextMenuUnderPointer
                (
                    data, data->wd_RootObject, imsg->MouseX, imsg->MouseY
                )
            )
            {
                iWin->Flags |= WFLG_RMBTRAP;
	    }
            else if (!data->wd_NoMenus)
            {
                iWin->Flags &= ~WFLG_RMBTRAP;
            }
	    is_handled = FALSE; /* fowardable to area event handlers */
	    break;

	case IDCMP_ACTIVEWINDOW:
	    data->wd_Flags |= MUIWF_ACTIVE;
	    set(oWin, MUIA_Window_Activate, TRUE);
	    break;

	case IDCMP_INACTIVEWINDOW:
	    data->wd_Flags &= ~MUIWF_ACTIVE;
	    set(oWin, MUIA_Window_Activate, FALSE);
	    set(oWin, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
	    break;

	case IDCMP_NEWSIZE:
	{
	    int hborders = iWin->BorderLeft + iWin->BorderRight;
	    int vborders = iWin->BorderTop  + iWin->BorderBottom;

	    /* set window limits according to window contents */
	    WindowLimits
            (
                iWin,
                data->wd_MinMax.MinWidth  + hborders,
                data->wd_MinMax.MinHeight + vborders,
                data->wd_MinMax.MaxWidth  + hborders,
                data->wd_MinMax.MaxHeight + vborders
            );
	}

	if ((iWin->GZZWidth  != data->wd_Width) || (iWin->GZZHeight != data->wd_Height))
	{
	    data->wd_Width  = iWin->GZZWidth;
	    data->wd_Height = iWin->GZZHeight;
	    DoMethod(data->wd_RootObject, MUIM_Hide);

	    if (data->wd_RenderInfo.mri_Window->Flags & WFLG_SIMPLE_REFRESH)
	    {
		data->wd_Flags |= MUIWF_RESIZING;
	    }
	    else
	    {
		_width(data->wd_RootObject) = data->wd_Width;
		_height(data->wd_RootObject) = data->wd_Height;
		DoMethod(data->wd_RootObject, MUIM_Layout);
		DoMethod(data->wd_RootObject, MUIM_Show);
		{
		    LONG left,top,width,height;

		    left = data->wd_RenderInfo.mri_Window->BorderLeft;
		    top = data->wd_RenderInfo.mri_Window->BorderTop,
			width = data->wd_RenderInfo.mri_Window->Width
			- data->wd_RenderInfo.mri_Window->BorderRight - left;
		    height = data->wd_RenderInfo.mri_Window->Height
			- data->wd_RenderInfo.mri_Window->BorderBottom - top;

//		    D(bug("%d:zune_imspec_draw(%p) l=%d t=%d w=%d h=%d xo=%d yo=%d\n",
//			  __LINE__, data->wd_Background, left, top, width,
//			  height, left, top));
		    zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
				     left, top, width, height, left, top, 0);
		}
		if (muiGlobalInfo(oWin)->mgi_Prefs->window_redraw == WINDOW_REDRAW_WITHOUT_CLEAR)
		    MUI_Redraw(data->wd_RootObject, MADF_DRAWOBJECT);
		else
		    MUI_Redraw(data->wd_RootObject, MADF_DRAWALL);
	    }
	}
	break;

	case IDCMP_REFRESHWINDOW:
	    if (data->wd_Flags & MUIWF_RESIZING)
	    {
		//LONG left,top,right,bottom;
		if (MUI_BeginRefresh(&data->wd_RenderInfo, 0))
		{
		    MUI_EndRefresh(&data->wd_RenderInfo, 0);
		}
		RefreshWindowFrame(data->wd_RenderInfo.mri_Window);

		data->wd_Flags &= ~MUIWF_RESIZING;
		_width(data->wd_RootObject) = data->wd_Width;
		_height(data->wd_RootObject) = data->wd_Height;
		DoMethod(data->wd_RootObject, MUIM_Layout);
		DoMethod(data->wd_RootObject, MUIM_Show);

		if (muiGlobalInfo(oWin)->mgi_Prefs->window_redraw == WINDOW_REDRAW_WITH_CLEAR)
		{
		    LONG left,top,width,height;

		    left = data->wd_RenderInfo.mri_Window->BorderLeft;
		    top = data->wd_RenderInfo.mri_Window->BorderTop,
			width = data->wd_RenderInfo.mri_Window->Width - data->wd_RenderInfo.mri_Window->BorderRight - left;
		    height = data->wd_RenderInfo.mri_Window->Height - data->wd_RenderInfo.mri_Window->BorderBottom - top;

		    if(data->wd_Flags & MUIWF_ERASEAREA)
		    {
//			D(bug("%d:zune_imspec_draw(%p) l=%d t=%d w=%d h=%d xo=%d yo=%d\n",
//			      __LINE__, data->wd_Background, left, top, width,
//			      height, left, top));
			zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
					 left, top, width, height, left, top, 0);
		    }
		    MUI_Redraw(data->wd_RootObject, MADF_DRAWALL);
		}
		else
		    MUI_Redraw(data->wd_RootObject, MADF_DRAWOBJECT);
	    }
	    else
	    {
		if (MUI_BeginRefresh(&data->wd_RenderInfo, 0))
		{
		    MUI_Redraw(data->wd_RootObject, MADF_DRAWALL);
		    MUI_EndRefresh(&data->wd_RenderInfo, 0);
		}
	    }
	    break;

	case IDCMP_CLOSEWINDOW:
	    set(oWin, MUIA_Window_CloseRequest, TRUE);
	    break;

	case IDCMP_MENUPICK:
	    if (data->wd_Menu)
	    {
		if (MENUNUM(imsg->Code != NOMENU) && ITEMNUM(imsg->Code) != NOITEM)
		{
		    struct MenuItem *item = ItemAddress(data->wd_Menu,imsg->Code);
		    if (item)
		    {
			Object *item_obj = (Object*)GTMENUITEM_USERDATA(item);
			if (item_obj)
			{
			    Object *app;
			    ULONG udata;

			    if (item->Flags & CHECKIT)
				set(item_obj, MUIA_Menuitem_Checked, !!(item->Flags & CHECKED));

			    set(item_obj, MUIA_Menuitem_Trigger, (IPTR)item);

			    get(oWin, MUIA_ApplicationObject, &app);
			    get(item_obj, MUIA_UserData, &udata);

			    set(app, MUIA_Application_MenuAction, udata);
			    set(oWin, MUIA_Window_MenuAction, udata);
			    DoMethod(app, MUIM_Application_ReturnID, udata);
			}
		    }
		}
	    }
	    break;

	case IDCMP_IDCMPUPDATE:
	    if (data->wd_VertProp || data->wd_HorizProp)
	    {
		struct TagItem *tag;
		tag = FindTagItem(GA_ID,(struct TagItem*)imsg->IAddress);
		if (tag)
		{
		    /* If there's a propclass object connected to the prop
		       gadget, the prop gadget's userdata will point to
		       that propclass object. See classes/prop.c */
		       
		    if (data->wd_VertProp)
		    {
			if (tag->ti_Data == GADGETID(data->wd_VertProp));
			
			if (tag->ti_Data == GADGETID(data->wd_UpButton))
			{
			    Object *prop = (Object *)((struct Gadget *)data->wd_VertProp)->UserData;
			    
			    if (prop) DoMethod(prop, MUIM_Prop_Decrease, 1);
			}			
			
			if (tag->ti_Data == GADGETID(data->wd_DownButton))
			{
			    Object *prop = (Object *)((struct Gadget *)data->wd_VertProp)->UserData;
			    
			    if (prop) DoMethod(prop, MUIM_Prop_Increase, 1);
			}
			
		    }

		    if (data->wd_HorizProp)
		    {
			if (tag->ti_Data == GADGETID(data->wd_HorizProp));
			
			if (tag->ti_Data == GADGETID(data->wd_LeftButton))
			{
			    Object *prop = (Object *)((struct Gadget *)data->wd_HorizProp)->UserData;
			    
			    if (prop) DoMethod(prop, MUIM_Prop_Decrease, 1);
			}
			
			if (tag->ti_Data == GADGETID(data->wd_RightButton))
			{
			    Object *prop = (Object *)((struct Gadget *)data->wd_HorizProp)->UserData;
			    
			    if (prop) DoMethod(prop, MUIM_Prop_Increase, 1);
			}
			
		    }
		}
	    }

	default:
	    is_handled = FALSE;
	    break;
    } /* switch (imsg->Class) */

    return is_handled;
}

static ULONG InvokeEventHandler (struct MUI_EventHandlerNode *ehn,
				 struct IntuiMessage *event, ULONG muikey)
{
    ULONG res;

    if (!(_flags(ehn->ehn_Object) & MADF_CANDRAW)) return 0;
    if (!(_flags(ehn->ehn_Object) & MADF_SHOWME)) return 0;

    if
    (
	event != NULL
	&& event->Class == IDCMP_MOUSEBUTTONS
	&& event->Code == SELECTDOWN 
	&& (_flags(ehn->ehn_Object) & MADF_INVIRTUALGROUP)
    )
    {
	/* 
            Here we filter out SELECTDOWN messages if objects is in a virtual 
            group but the click went out of the virtual group 
        */
    	Object *obj = ehn->ehn_Object;
    	Object *parent = obj;
    	Object *wnd = _win(obj);

	while (get(parent,MUIA_Parent,&parent))
	{
	    if (!parent) break;
	    if (wnd == parent) break;
	    if (_flags(parent) & MADF_ISVIRTUALGROUP)
	    {
		if
                (
                       event->MouseX < _mleft(parent) 
                    || event->MouseX > _mright(parent) 
                    || event->MouseY < _mtop(parent) 
                    || event->MouseY > _mbottom(parent)
                )
		{
		    return 0;
		}
	    }
	}

    }

    if (ehn->ehn_Flags & MUI_EHF_HANDLEINPUT)
    {
    	DoMethod(ehn->ehn_Object, MUIM_HandleInput, (IPTR)event, muikey);
    	res = 0;
    }
    else
    {
	if (ehn->ehn_Class)
	    res = CoerceMethod
            (
                ehn->ehn_Class, ehn->ehn_Object, MUIM_HandleEvent, 
                (IPTR)event, muikey
            );
	else
	    res = DoMethod(ehn->ehn_Object, MUIM_HandleEvent, (IPTR)event, muikey);
    }
    return res;
}

static void HandleRawkey(Object *win, struct MUI_WindowData *data, 
			     struct IntuiMessage *event)
{
    struct MinNode              *mn;
    struct MUI_EventHandlerNode *ehn;
    ULONG                        res;
    LONG                         muikey = MUIKEY_NONE;
    Object                      *active_object = NULL;
    IPTR                         disabled;

    /* check if imsg translate to predefined keystroke */
    {
	struct InputEvent ievent;
	BOOL matched = FALSE;

	ievent.ie_NextEvent    = NULL;
	ievent.ie_Class        = IECLASS_RAWKEY;
	ievent.ie_SubClass     = 0;
	ievent.ie_Code         = event->Code;
	ievent.ie_Qualifier    = event->Qualifier;
	ievent.ie_EventAddress = (APTR *) *((ULONG *)(event->IAddress));

	for (muikey = MUIKEY_COUNT - 1; muikey >= MUIKEY_PRESS; muikey--)
	{
	    if (muiGlobalInfo(win)->mgi_Prefs->muikeys[muikey].ix_well != NULL
		&& MatchIX(&ievent, &muiGlobalInfo(win)->mgi_Prefs->muikeys[muikey].ix))
	    {
		matched = TRUE;
		break;
	    }
	}

	if (matched)
	{
	    if (muikey == MUIKEY_PRESS && (event->Code & IECODE_UP_PREFIX))
		muikey = MUIKEY_RELEASE;
	}
	else
	{
	    muikey = MUIKEY_NONE;
	}
    } /* check if imsg translate to predefined keystroke */

    active_object = data->wd_ActiveObject;
    if (active_object != NULL)
	get(active_object, MUIA_Disabled, &disabled);

    /* try ActiveObject */
    if ((active_object != NULL) && !disabled)
    {
#if 0
	/* sba:
	** Which method should be used for muikeys? MUIM_HandleInput or
	** MUIM_HandleEvent. Also note that there is a flag MUI_EHF_ALWAYSKEYS
	** which probably means that all keys events are requested??
	** For now MUIM_HandleEvent is used as this is currently implemented
	** in Area class ;) although I guess it should be MUIM_HandleInput as this
	** was earlier
	*/

	if (muikey != MUIKEY_NONE)
	{
	    res = DoMethod(data->wd_ActiveObject, MUIM_HandleEvent, (IPTR)event, muikey);
	    if (res & MUI_EventHandlerRC_Eat) return;
	}
#endif
	for (mn = data->wd_EHList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
	{
	    ehn = (struct MUI_EventHandlerNode *)mn;

	    if ((ehn->ehn_Object == active_object)
		&& ((ehn->ehn_Events & IDCMP_RAWKEY)
		    || ((muikey != MUIKEY_NONE)
			&& (ehn->ehn_Flags & MUI_EHF_ALWAYSKEYS))))
	    {
		res = InvokeEventHandler(ehn, event, muikey);
		if (res & MUI_EventHandlerRC_Eat)
		    return;

		/* Leave the loop if a different object has been activated */
		if (active_object != data->wd_ActiveObject)
		    break;
	    }
	} /* for (mn = data->wd_EHList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ) */

    } /* if (active_object && !disabled) */

    /* try DefaultObject */
    if (data->wd_DefaultObject != NULL)
	get(data->wd_DefaultObject, MUIA_Disabled, &disabled);

    if ((data->wd_DefaultObject != NULL) && !disabled
	&& (active_object != data->wd_DefaultObject))
    {
    	/* No, we only should do this if the object actually has requested this via RequestIDCMP()! */
//    	if (muikey != MUIKEY_NONE && (_flags(data->wd_DefaultObject) & MADF_CANDRAW))
//    	{
//	    DoMethod(data->wd_DefaultObject, MUIM_HandleInput, event, muikey);
//	    return;
//    	}

	for (mn = data->wd_EHList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
	{
	    ehn = (struct MUI_EventHandlerNode *)mn;

	    if ((ehn->ehn_Object == data->wd_DefaultObject) 
		&& (ehn->ehn_Events & IDCMP_RAWKEY))
	    {
		res = InvokeEventHandler(ehn, event, muikey);
		if (res & MUI_EventHandlerRC_Eat)
		    return;
	    }
	}

    } /* if ... default object */

    /* try Control Chars */
    {
    	struct IntuiMessage imsg;
    	ULONG key;

	/* Remove the up prefix as convert key does not convert a upkey event */
    	imsg = *event;
    	imsg.Code &= ~IECODE_UP_PREFIX;
    	key = ConvertKey(&imsg);

    	if (key)
    	{
            for (mn = data->wd_CCList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
            {
		ehn = (struct MUI_EventHandlerNode *)mn;

		if (ehn->ehn_Events == key)
		{
		    IPTR disabled;
		    LONG muikey2 = ehn->ehn_Flags;

		    get(ehn->ehn_Object, MUIA_Disabled, &disabled);
		    if (disabled)
			continue;

		    if (event->Code & IECODE_UP_PREFIX)
		    {
			/* simulate a release */
			if (muikey2 == MUIKEY_PRESS)
			    muikey2 = MUIKEY_RELEASE;
			else
			    return;
		    }

		    if ((muikey2 != MUIKEY_NONE)
			&& (_flags(ehn->ehn_Object) & MADF_CANDRAW)
			&& (_flags(ehn->ehn_Object) & MADF_SHOWME))
		    {
			res = CoerceMethod
			    (
				ehn->ehn_Class, ehn->ehn_Object, MUIM_HandleEvent, 
				NULL, muikey2
				);
			if (res & MUI_EventHandlerRC_Eat)
			    return;
		    }
		}
	    }
	}
    } /* try control chars */

    if ((muikey != MUIKEY_NONE)
	&& !(data->wd_DisabledKeys & (1<<muikey)))
    {
	/* nobody has eaten the message so we can try ourself */
	switch (muikey)
	{
	    case MUIKEY_PRESS: break;
	    case MUIKEY_TOGGLE: break;
	    case MUIKEY_UP: break;
	    case MUIKEY_DOWN: break;
	    case MUIKEY_PAGEUP: break;
	    case MUIKEY_PAGEDOWN: break;
	    case MUIKEY_TOP: break;
	    case MUIKEY_BOTTOM: break;
	    case MUIKEY_LEFT: break;
	    case MUIKEY_RIGHT: break;
	    case MUIKEY_WORDLEFT: break;
	    case MUIKEY_WORDRIGHT: break;
	    case MUIKEY_LINESTART: break;
	    case MUIKEY_LINEEND: break;
	    case MUIKEY_GADGET_NEXT:
		set(win, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next);
		break;
	    case MUIKEY_GADGET_PREV:
		set(win, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Prev);
		break;
	    case MUIKEY_GADGET_OFF:
		set(win, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
		break;
	    case MUIKEY_WINDOW_CLOSE:
		set(win, MUIA_Window_CloseRequest, TRUE);
		break;
	    case MUIKEY_WINDOW_NEXT: break;
	    case MUIKEY_WINDOW_PREV: break;
	    case MUIKEY_HELP: break;
	    case MUIKEY_POPUP: break;
	    default: break;
	}
    }
}

/* forward non-keystroke events to event handlers */
static void HandleInputEvent(Object *win, struct MUI_WindowData *data, 
			     struct IntuiMessage *event)
{
    struct MinNode              *mn;
    struct MUI_EventHandlerNode *ehn;
    ULONG                        res;
    ULONG                        mask = event->Class;

    for (mn = data->wd_EHList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
    {
	ehn = (struct MUI_EventHandlerNode *)mn;
	    
	if (ehn->ehn_Events & mask)
	{
	    IPTR disabled;

	    get(ehn->ehn_Object, MUIA_Disabled, &disabled);
	    if (disabled)
		continue;

	    res = InvokeEventHandler(ehn, event, MUIKEY_NONE);
	    if (res & MUI_EventHandlerRC_Eat)
		return;
	}
    }
}


/* process window message, this does a ReplyMsg() to the message */
/* Called from application.c */
void _zune_window_message(struct IntuiMessage *imsg)
{
    struct Window *iWin;
    Object        *oWin;
    struct MUI_WindowData *data;
    struct IntuiMessage imsg_copy;
    BOOL handled;

    iWin = imsg->IDCMPWindow;
    oWin = (Object *)iWin->UserData;
    data = muiWindowData(oWin);

    imsg_copy = *imsg;
    ReplyMsg((struct Message*)imsg);

    if (data->wd_DragObject)
    {
	HandleDragging(oWin, data, &imsg_copy);
	return;
    }

    handled = HandleWindowEvent(oWin, data, &imsg_copy);
    if (!handled)
    {
	if (IDCMP_RAWKEY == imsg->Class)
	    HandleRawkey(oWin, data, &imsg_copy);
	else
	    HandleInputEvent(oWin, data, &imsg_copy);
    }
}

/******************************************************************************/
/******************************************************************************/

/* code for setting MUIA_Window_RootObject */
static void window_change_root_object (struct MUI_WindowData *data, Object *obj,
			   Object *newRoot)
{
    Object *oldRoot;

    ASSERT_VALID_PTR(data);
    ASSERT_VALID_PTR(obj);

    oldRoot = data->wd_RootObject;
    if (!(data->wd_Flags & MUIWF_OPENED))
    {
	if (oldRoot)
	{
	    if (data->wd_ActiveObject == oldRoot)
		set(obj, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
	    DoMethod(oldRoot, MUIM_DisconnectParent);
	}

	data->wd_RootObject = newRoot;
	if (newRoot)
	{
	    /* if window is in App tree, inform child */
	    if (muiNotifyData(obj)->mnd_GlobalInfo)
		DoMethod(newRoot, MUIM_ConnectParent, (IPTR)obj);
	}
    }
}

static struct ObjNode *FindObjNode(struct MinList *list, Object *obj)
{
    struct ObjNode *node;

    ASSERT_VALID_PTR(list);

    if (!obj)
	return NULL;

    ASSERT_VALID_PTR(obj);

    for (node = (struct ObjNode*)list->mlh_Head; node->node.mln_Succ; node = (struct ObjNode*)node->node.mln_Succ)
    {
    	if (node->obj == obj)
	{
	    return node;
	}
    }
    return NULL;
}

static Object *GetFirstActiveObject (struct MUI_WindowData *data)
{
    ASSERT_VALID_PTR(data);

    if (!IsListEmpty((struct List*)&data->wd_CycleChain))
	return ((struct ObjNode*)data->wd_CycleChain.mlh_Head)->obj;
    else
	return NULL;
}

static Object *GetLastActiveObject (struct MUI_WindowData *data)
{
    ASSERT_VALID_PTR(data);

    if (!IsListEmpty((struct List*)&data->wd_CycleChain))
	return ((struct ObjNode*)data->wd_CycleChain.mlh_TailPred)->obj;
    else
	return NULL;
}

typedef struct ObjNode *objnode_iterator_t(struct ObjNode *curr_node);

static objnode_iterator_t NextObjNodeIterator;
static objnode_iterator_t PrevObjNodeIterator;

static struct ObjNode *NextObjNodeIterator (struct ObjNode *curr_node)
{
    if (curr_node->node.mln_Succ->mln_Succ)
	return (struct ObjNode*)curr_node->node.mln_Succ;
    else
	return NULL;
}

static struct ObjNode *PrevObjNodeIterator (struct ObjNode *curr_node)
{
    if (curr_node->node.mln_Pred->mln_Pred)
	return (struct ObjNode*)curr_node->node.mln_Pred;
    else
	return NULL;
}

static Object *GetPrevNextActiveObject (struct ObjNode *old_activenode, objnode_iterator_t node_iterator)
{
    struct ObjNode *curr_node;
    struct ObjNode *node;
    Object *obj;

    ASSERT_VALID_PTR(old_activenode);
    
    curr_node = old_activenode;
    node = NULL;
    obj = NULL;

    while (curr_node)
    {
	node = node_iterator(curr_node);

	if (node)
	    obj = node->obj;

	/* let's see if this obj meets cycle requirements (enabled & visible) */
	if (obj)
	{
	    IPTR is_disabled;

	    get(obj, MUIA_Disabled, &is_disabled);

	    if (!is_disabled && (_flags(obj) & MADF_CANDRAW))
	    {
		return obj;
	    }
	}

	curr_node = node;
	obj = NULL;
	node = NULL;
    }
    return obj;
}


/**************************************************************************
 Code for setting MUIA_Window_ActiveObject
 If the current active object is not in a cycle chain it handles
 MUIV_Window_ActiveObject_Next and MUIV_Window_ActiveObject_Prev
 currently as there is no active object
**************************************************************************/
static void window_set_active_object (struct MUI_WindowData *data, Object *obj, ULONG newval)
{
    struct ObjNode *old_activenode;
    Object *old_active;

    ASSERT_VALID_PTR(data);
    ASSERT_VALID_PTR(obj);

    if ((ULONG)data->wd_ActiveObject == newval)
	return;

    old_active = data->wd_ActiveObject;
    old_activenode = FindObjNode(&data->wd_CycleChain, old_active);
    data->wd_ActiveObject = NULL;
    if ((old_active != NULL) && (_flags(old_active) & MADF_CANDRAW))
	DoMethod(old_active, MUIM_GoInactive);

    switch (newval)
    {
	case MUIV_Window_ActiveObject_None:
	    break;

	case MUIV_Window_ActiveObject_Next:
	    if (old_activenode != NULL)
		data->wd_ActiveObject = GetPrevNextActiveObject(old_activenode,
								NextObjNodeIterator);
	    if (NULL == data->wd_ActiveObject)
		data->wd_ActiveObject = GetFirstActiveObject(data);
	    break;

	case MUIV_Window_ActiveObject_Prev:
	    if (old_activenode)
		data->wd_ActiveObject = GetPrevNextActiveObject(old_activenode,
								PrevObjNodeIterator);
	    if (NULL == data->wd_ActiveObject)
		data->wd_ActiveObject = GetLastActiveObject(data);
	    break;

	default:
	    data->wd_ActiveObject = (Object*)newval;
	    break;
    }

    if (data->wd_ActiveObject != NULL)
    {
	if (_flags(data->wd_ActiveObject) & MADF_CANDRAW)
	    DoMethod(data->wd_ActiveObject, MUIM_GoActive);
    }
}


/*
 * calculate real dimensions from programmer requirements.
 * may be overridden by user settings if MUIA_Window_ID is set.
 */
/* MUIV_Window_Height_Screen and MUIV_Window_Height_Visible
 * are not handled yet, as their Width couterparts.
 */
static void window_select_dimensions (struct MUI_WindowData *data)
{
    if (!data->wd_Width)
    {
	if (data->wd_ReqWidth > 0) data->wd_Width = data->wd_ReqWidth;
	else if (data->wd_ReqWidth == MUIV_Window_Width_Default)
	    data->wd_Width = data->wd_MinMax.DefWidth;
	else if (_between(MUIV_Window_Width_MinMax(100),
			  data->wd_ReqWidth,
			  MUIV_Window_Width_MinMax(0)))
	{
	    data->wd_Width = data->wd_MinMax.MinWidth
		- data->wd_ReqWidth
		* (data->wd_MinMax.MaxWidth - data->wd_MinMax.MinWidth);
	}
	else if (_between(MUIV_Window_Width_Screen(100),
			  data->wd_ReqWidth,
			  MUIV_Window_Width_Screen(0)))
	{
	    data->wd_Width = data->wd_RenderInfo.mri_ScreenWidth
		* (- (data->wd_ReqWidth + 200)) / 100;
	}
	else if (_between(MUIV_Window_Width_Visible(100),
			  data->wd_ReqWidth,
			  MUIV_Window_Width_Visible(0)))
	{
	    data->wd_Width = data->wd_RenderInfo.mri_ScreenWidth
		* (- (data->wd_ReqWidth + 100)) / 100;
	}

	if (data->wd_ReqHeight > 0) data->wd_Height = data->wd_ReqHeight;
	else if (data->wd_ReqHeight == MUIV_Window_Height_Default)
	    data->wd_Height = data->wd_MinMax.DefHeight;
	else if (_between(MUIV_Window_Height_MinMax(100),
			  data->wd_ReqHeight,
			  MUIV_Window_Height_MinMax(0)))
	{
	    data->wd_Height = data->wd_MinMax.MinHeight
		- data->wd_ReqHeight
		* (data->wd_MinMax.MaxHeight - data->wd_MinMax.MinHeight);
	}
	else if (_between(MUIV_Window_Height_Screen(100),
			  data->wd_ReqHeight,
			  MUIV_Window_Height_Screen(0)))
	{
	    struct Screen *scr;
	    int height;

	    scr = data->wd_RenderInfo.mri_Screen;

	    height = scr->Height - data->wd_RenderInfo.mri_BorderTop - data->wd_RenderInfo.mri_BorderBottom;

	    /* This is new to Zune: If TopEdge Delta is requested
	     * the screenheight doesn't cover the barlayer */
	    if (data->wd_Y <= MUIV_Window_TopEdge_Delta(0))
		height -= scr->BarHeight + 1;

	    data->wd_Height = height * (- (data->wd_ReqHeight + 200)) / 100;
	}
	else if (_between(MUIV_Window_Height_Visible(100),
			  data->wd_ReqHeight,
			  MUIV_Window_Height_Visible(0)))
	{
	    data->wd_Height = data->wd_RenderInfo.mri_ScreenHeight
		* (- (data->wd_ReqHeight + 100)) / 100;
	}

	/* scaled */
	if (data->wd_ReqWidth == MUIV_Window_Width_Scaled)
	    data->wd_Width = data->wd_Height * data->wd_MinMax.MinWidth
		/ data->wd_MinMax.MinHeight;
	else if (data->wd_ReqHeight == MUIV_Window_Width_Scaled)
	    data->wd_Height = data->wd_Width * data->wd_MinMax.MinHeight
		/ data->wd_MinMax.MinWidth;
    }
    data->wd_Width = CLAMP(data->wd_Width, data->wd_MinMax.MinWidth,
			   data->wd_MinMax.MaxWidth);
    data->wd_Height = CLAMP(data->wd_Height, data->wd_MinMax.MinHeight,
			    data->wd_MinMax.MaxHeight);
}


/**************************************************************************
 OM_NEW
**************************************************************************/
static ULONG Window_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct MUI_WindowData *data;
    struct TagItem *tags,*tag;

    obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg);
    if (!obj)
	return FALSE;

    /* Initial local instance data */
    data = INST_DATA(cl, obj);
    data->wd_RenderInfo.mri_WindowObject = obj;

    NewList((struct List*)&(data->wd_EHList));
    NewList((struct List*)&(data->wd_CCList));
    NewList((struct List*)&(data->wd_CycleChain));
    NewList((struct List*)&(data->wd_IDList));

    data->wd_CrtFlags = WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET 
                      | WFLG_CLOSEGADGET | WFLG_SIMPLE_REFRESH 
                      | WFLG_REPORTMOUSE | WFLG_NEWLOOKMENUS;

    data->wd_Events = _zune_window_get_default_events();
    data->wd_ActiveObject = NULL;
    data->wd_ID = 0;
    data->wd_ReqHeight = MUIV_Window_Height_Default;
    data->wd_ReqWidth = MUIV_Window_Width_Default;
    data->wd_RootObject = NULL;
    data->wd_DefaultObject = NULL;

/* alternate dimensions */
/* no change in coordinates */
    data->wd_AltDim.Top = MUIV_Window_AltTopEdge_NoChange;
    data->wd_AltDim.Left = MUIV_Window_AltLeftEdge_NoChange;
/* default to min size */
    data->wd_AltDim.Width = MUIV_Window_AltWidth_MinMax(0);
    data->wd_AltDim.Height = MUIV_Window_AltHeight_MinMax(0);
    data->wd_X = MUIV_Window_LeftEdge_Centered;
    data->wd_Y = MUIV_Window_TopEdge_Centered;
    data->wd_DisabledKeys = 0L;

    /* parse initial taglist */

    for (tags = msg->ops_AttrList; (tag = NextTagItem((struct TagItem **)&tags)); )
    {
	switch (tag->ti_Tag)
	{
	    case MUIA_Window_EraseArea:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_ERASEAREA);
		break;

	    case MUIA_Window_CloseGadget:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_CLOSEGADGET);
		break;

	    case MUIA_Window_SizeGadget:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_SIZEGADGET);
		break;

	    case MUIA_Window_Backdrop:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_BACKDROP);
		break;

	    case MUIA_Window_Borderless:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_BORDERLESS);
		break;

	    case MUIA_Window_DepthGadget:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_DEPTHGADGET);
		break;

	    case MUIA_Window_DragBar:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_DRAGBAR);
		break;

	    case MUIA_Window_SizeRight:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_SIZEBRIGHT);
		break;

	    case  MUIA_Window_Height:
		data->wd_ReqHeight = (LONG)tag->ti_Data;
		break;

	    case MUIA_Window_Width:
		data->wd_ReqWidth = (LONG)tag->ti_Data;
		break;

	    case MUIA_Window_ID:
		set(obj, MUIA_Window_ID, tag->ti_Data);
		break;

	    case MUIA_Window_IsSubWindow:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_ISSUBWINDOW);
		break;

	    case MUIA_Window_Title:
		set(obj, MUIA_Window_Title, tag->ti_Data);
		break;

	    case MUIA_Window_ScreenTitle:
		set(obj, MUIA_Window_ScreenTitle, tag->ti_Data);
		break;

	    case MUIA_Window_Activate:
		_handle_bool_tag(data->wd_Flags, !tag->ti_Data, MUIWF_DONTACTIVATE);
		break;

	    case MUIA_Window_DefaultObject:
		set(obj, MUIA_Window_DefaultObject, tag->ti_Data);
		break;

	    case MUIA_Window_Menustrip:
		data->wd_ChildMenustrip = (Object*)tag->ti_Data;
		break;

   	    case MUIA_Window_NoMenus:
                data->wd_NoMenus = (BOOL) tag->ti_Data;
                break;
            
            case MUIA_Window_RootObject:
		if (!tag->ti_Data)
		{
		    CoerceMethod(cl, obj, OM_DISPOSE);
		    return 0;
		}
		set(obj, MUIA_Window_RootObject, tag->ti_Data);
		break;

	    case MUIA_Window_AltHeight:
		data->wd_AltDim.Height = (WORD)tag->ti_Data;
		break;

	    case MUIA_Window_AltWidth:
		data->wd_AltDim.Width = (WORD)tag->ti_Data;
		break;

	    case MUIA_Window_AltLeftEdge:
		data->wd_AltDim.Left = (WORD)tag->ti_Data;
		break;

	    case MUIA_Window_AltTopEdge:
		data->wd_AltDim.Top = (WORD)tag->ti_Data;
		break;

	    case MUIA_Window_AppWindow:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_ISAPPWINDOW);
		break;

	    case MUIA_Window_LeftEdge:
		data->wd_X = tag->ti_Data;
		break;

	    case MUIA_Window_TopEdge:
		data->wd_Y = tag->ti_Data;
		break;

	    case MUIA_Window_UseBottomBorderScroller:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_USEBOTTOMSCROLLER);
		break;

	    case MUIA_Window_UseRightBorderScroller:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_USERIGHTSCROLLER);
		break;

	    case MUIA_Window_DisableKeys:
		data->wd_DisabledKeys = tag->ti_Data;
		break;

	    case MUIA_Window_RefWindow:
		data->wd_RefWindow = (Object *)tag->ti_Data;
		break;
	}
    }

/*      D(bug("muimaster.library/window.c: Window Object created at 0x%lx back=%lx\n", */
/*  	  obj,data->wd_Background)); */

    return (ULONG)obj;
}

/**************************************************************************
 OM_DISPOSE
**************************************************************************/
static ULONG Window_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

/*      D(bug("Window_Dispose(%p)\n", obj)); */
    if (muiGlobalInfo(obj) && _app(obj))
    {
/*  	D(bug(" Window_Dispose(%p) : calling app->OM_REMMEMBER\n", obj)); */
	DoMethod(_app(obj), OM_REMMEMBER, (IPTR)obj);
    }
    if (data->wd_RootObject)
	MUI_DisposeObject(data->wd_RootObject);

    if (data->wd_Title)
	FreeVec(data->wd_Title);

    if (data->wd_ScreenTitle)
	FreeVec(data->wd_ScreenTitle);

/*      D(bug(" Window_Dispose(%p) : calling supermethod\n", obj)); */
    return DoSuperMethodA(cl, obj, msg);
}

/**************************************************************************
 OM_SET
**************************************************************************/
static ULONG Window_Set(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    struct TagItem        *tags = msg->ops_AttrList;
    struct TagItem        *tag;

    while ((tag = NextTagItem((struct TagItem **)&tags)) != NULL)
    {
	switch (tag->ti_Tag)
	{
	    case MUIA_Window_Activate:
		if (data->wd_RenderInfo.mri_Window)
		{
		    if (tag->ti_Data && !(data->wd_Flags & MUIWF_ACTIVE))
		    {
			ActivateWindow(data->wd_RenderInfo.mri_Window);
			_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_ACTIVE);
		    }
		}
		else
		    _handle_bool_tag(data->wd_Flags, !tag->ti_Data, MUIWF_DONTACTIVATE);
		break;

	    case MUIA_Window_ActiveObject:
/*  		D(bug("MUIA_Window_ActiveObject %ld (%p)\n", tag->ti_Data, tag->ti_Data)); */
		window_set_active_object(data, obj, tag->ti_Data);
		break;
	    case MUIA_Window_DefaultObject:
		data->wd_DefaultObject = (APTR)tag->ti_Data;
		break;
	    case MUIA_Window_ID:
		data->wd_ID = tag->ti_Data;
		break;
	    case MUIA_Window_IsSubWindow:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_ISSUBWINDOW);
		break;
	    case MUIA_Window_Open:
		if (tag->ti_Data && !(data->wd_Flags & MUIWF_OPENED))
		    window_Open(cl, obj);
		else if (!tag->ti_Data && (data->wd_Flags & MUIWF_OPENED))
		    window_Close(cl, obj);
		else if (tag->ti_Data && (data->wd_Flags & MUIWF_OPENED))
		{
		    DoMethod(obj, MUIM_Window_ToFront);
		    set(obj, MUIA_Window_Activate, TRUE);
		}
		break;
	    case MUIA_Window_RootObject:
		window_change_root_object(data, obj, (Object *)tag->ti_Data);
		break;

	    case MUIA_Window_Title:
		if (data->wd_Title) FreeVec(data->wd_Title);
		data->wd_Title = StrDup((STRPTR)tag->ti_Data);
		if (data->wd_RenderInfo.mri_Window)
		    SetWindowTitles(data->wd_RenderInfo.mri_Window,data->wd_Title, (CONST_STRPTR)~0);
		break;

	    case MUIA_Window_ScreenTitle:
		if (data->wd_ScreenTitle) FreeVec(data->wd_ScreenTitle);
		data->wd_ScreenTitle = StrDup((STRPTR)tag->ti_Data);
		if (data->wd_RenderInfo.mri_Window)
		    SetWindowTitles(data->wd_RenderInfo.mri_Window,
				    (CONST_STRPTR)~0, data->wd_ScreenTitle);
		break;
            
            case MUIA_Window_NoMenus:
                data->wd_NoMenus = (BOOL) tag->ti_Data;
                break;
                
	    case MUIA_Window_UseBottomBorderScroller:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_USEBOTTOMSCROLLER);
		break;

	    case MUIA_Window_UseRightBorderScroller:
		_handle_bool_tag(data->wd_Flags, tag->ti_Data, MUIWF_USERIGHTSCROLLER);
		break;

	    case MUIA_Window_DisableKeys:
		data->wd_DisabledKeys = tag->ti_Data;
		break;

	    case MUIA_Window_RefWindow:
		data->wd_RefWindow = (Object *)tag->ti_Data;
		break;

	    case MUIA_Window_WandererBackdrop:
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_BACKDROP);
		_handle_bool_tag(data->wd_CrtFlags, !tag->ti_Data, WFLG_DRAGBAR);
		_handle_bool_tag(data->wd_CrtFlags, !tag->ti_Data, WFLG_SIZEGADGET);
		_handle_bool_tag(data->wd_CrtFlags, !tag->ti_Data, WFLG_CLOSEGADGET);
		_handle_bool_tag(data->wd_CrtFlags, !tag->ti_Data, WFLG_DEPTHGADGET);
		_handle_bool_tag(data->wd_CrtFlags, tag->ti_Data, WFLG_BORDERLESS);
		if (tag->ti_Data)
		{
		    data->wd_ReqWidth = (LONG)MUIV_Window_Width_Screen(100);
		    /* won't take the barlayer into account */
		    data->wd_ReqHeight = (LONG)MUIV_Window_Height_Screen(100);
		    data->wd_Width = 0;
		    data->wd_Height = 0;
		    data->wd_X = (LONG)0;
		    /* place the window below the bar layer */
		    data->wd_Y = (LONG)MUIV_Window_TopEdge_Delta(0);
		}
		break;
	}
    }

    return DoSuperMethodA(cl, obj, (Msg)msg);
}

/**************************************************************************
 OM_GET
**************************************************************************/
static ULONG Window_Get(struct IClass *cl, Object *obj, struct opGet *msg)
{
#define STORE *(msg->opg_Storage)

    struct MUI_WindowData *data = INST_DATA(cl, obj);

    STORE = (IPTR)0;

    switch(msg->opg_AttrID)
    {
	case MUIA_Window_Activate:
	    if (data->wd_Flags & MUIWF_OPENED)
		STORE = (data->wd_Flags & MUIWF_ACTIVE) ? TRUE : FALSE;
	    else
		STORE = FALSE;
	    return(TRUE);
        case MUIA_Window_Window:
            STORE = (data->wd_Flags & MUIWF_OPENED) ? ((ULONG)data->wd_RenderInfo.mri_Window) : FALSE;
            return 1;
	case MUIA_Window_ActiveObject:
	    STORE = (ULONG)data->wd_ActiveObject;
	    return 1;
	case MUIA_Window_CloseRequest:
	    STORE = FALSE;
	    return(TRUE);
	case MUIA_Window_DefaultObject:
	    STORE = (ULONG)data->wd_DefaultObject;
	    return(TRUE);
	case MUIA_Window_DisableKeys:
	    STORE = data->wd_DisabledKeys;
	    break;
	case MUIA_Window_Height:
	    STORE = (ULONG)data->wd_Height;
	    return(TRUE);
	case MUIA_Window_ID:
	    STORE = data->wd_ID;
	    return(TRUE);
	case MUIA_Window_IsSubWindow:
	    STORE = (data->wd_Flags & MUIWF_ISSUBWINDOW) ? TRUE : FALSE;
	    return(TRUE);
	case MUIA_Window_LeftEdge:
	    if (data->wd_RenderInfo.mri_Window)
		STORE = (ULONG)data->wd_RenderInfo.mri_Window->LeftEdge;
	    else
		STORE = (ULONG)0;
	    return(TRUE);
	case MUIA_Window_Open:
	    STORE = (data->wd_Flags & MUIWF_OPENED) ? TRUE : FALSE;
	    return(TRUE);
	case MUIA_Window_RootObject:
	    STORE = (ULONG)data->wd_RootObject;
	    return(TRUE);
	case MUIA_Window_ScreenTitle:
	    STORE = (ULONG)data->wd_ScreenTitle;
	    return(TRUE);
	case MUIA_Window_Title:
	    STORE = (ULONG)data->wd_Title;
	    return(TRUE);
	case MUIA_Window_TopEdge:
	    if (data->wd_RenderInfo.mri_Window)
		STORE = (ULONG)data->wd_RenderInfo.mri_Window->TopEdge;
	    else
		STORE = (ULONG)0;
	    return(TRUE);
	case MUIA_Window_Width:
	    STORE = (ULONG)data->wd_Width;
	    return(TRUE);
	case MUIA_Version:
	    STORE = __version;
	    return(TRUE);
	case MUIA_Revision:
	    STORE = __revision;
	    return(TRUE);
    }

    return DoSuperMethodA(cl, obj, (Msg) msg);
#undef STORE
}


/**************************************************************************
 Called by Application (parent) object whenever this object is added.
 init GlobalInfo
**************************************************************************/
static ULONG Window_ConnectParent(struct IClass *cl, Object *obj,
		     struct MUIP_ConnectParent *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    if (!DoSuperMethodA(cl,obj,(Msg)msg)) return 0;

    if (data->wd_RootObject)
	DoMethod(data->wd_RootObject, MUIM_ConnectParent, (IPTR)obj);

    if (data->wd_ChildMenustrip)
	DoMethod(data->wd_ChildMenustrip, MUIM_ConnectParent, (IPTR)obj);

    return TRUE;
}


/**************************************************************************
 called by parent object
**************************************************************************/
static ULONG Window_DisconnectParent(struct IClass *cl, Object *obj, struct MUIP_DisconnectParent *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

/*      D(bug("Window_DisconnectParent(%p) : muiGlobalInfo=%p\n", muiGlobalInfo(obj))); */
    if (muiGlobalInfo(obj))
    {
	/* Close the window before disconnecting all the childs */
	if ((data->wd_Flags & MUIWF_OPENED))
	{
/*  	    D(bug(" Window_DisconnectParent(%p) : closing window\n", muiGlobalInfo(obj))); */
	    set(obj, MUIA_Window_Open, FALSE);
	}
	if (data->wd_ChildMenustrip)
	    DoMethod(data->wd_ChildMenustrip, MUIM_DisconnectParent, (IPTR)obj);

	if (data->wd_RootObject)
	    DoMethodA(data->wd_RootObject, (Msg)msg);
	
/*  	D(bug(" Window_DisconnectParent(%p) : calling supermethod\n", muiGlobalInfo(obj))); */
	return DoSuperMethodA(cl,obj,(Msg)msg);
    }
    else
    {
	return 0;
    }
}

/*
 * Called before window is opened or resized. It determines its bounds,
 * so you can call window_select_dimensions() to find the final dims.
 */
static void window_minmax(Object *obj, struct MUI_WindowData *data)
{
    UWORD wd_innerLeft, wd_innerRight, wd_innerTop, wd_innerBottom;

    if (data->wd_CrtFlags & WFLG_BORDERLESS)
    {
	wd_innerLeft   = 0;
	wd_innerRight  = 0;
	wd_innerTop    = 0;
	wd_innerBottom = 0;
    }
    else
    {
	wd_innerLeft   = muiGlobalInfo(obj)->mgi_Prefs->window_inner_left;
	wd_innerRight  = muiGlobalInfo(obj)->mgi_Prefs->window_inner_right;
	wd_innerTop    = muiGlobalInfo(obj)->mgi_Prefs->window_inner_top;
	wd_innerBottom = muiGlobalInfo(obj)->mgi_Prefs->window_inner_bottom;
    }

    if (!(muiAreaData(data->wd_RootObject)->mad_Flags & MADF_INNERLEFT))
    {
	muiAreaData(data->wd_RootObject)->mad_Flags |= MADF_INNERLEFT;
    	muiAreaData(data->wd_RootObject)->mad_HardILeft = wd_innerLeft;
    }

    if (!(muiAreaData(data->wd_RootObject)->mad_Flags & MADF_INNERTOP))
    {
	muiAreaData(data->wd_RootObject)->mad_Flags |= MADF_INNERTOP;
    	muiAreaData(data->wd_RootObject)->mad_HardITop = wd_innerTop;
    }

    if (!(muiAreaData(data->wd_RootObject)->mad_Flags & MADF_INNERRIGHT))
    {
	muiAreaData(data->wd_RootObject)->mad_Flags |= MADF_INNERRIGHT;
    	muiAreaData(data->wd_RootObject)->mad_HardIRight = wd_innerRight;
    }

    if (!(muiAreaData(data->wd_RootObject)->mad_Flags & MADF_INNERBOTTOM))
    {
	muiAreaData(data->wd_RootObject)->mad_Flags |= MADF_INNERBOTTOM;
    	muiAreaData(data->wd_RootObject)->mad_HardIBottom = wd_innerBottom;
    }

    /* inquire about sizes */
    DoMethod(data->wd_RootObject, MUIM_AskMinMax, (ULONG)&data->wd_MinMax);
/*      D(bug("*** root minmax = %ld,%ld => %ld,%ld\n", data->wd_MinMax.MinWidth, */
/*  	  data->wd_MinMax.MinHeight, */
/*  	  data->wd_MinMax.MaxWidth, data->wd_MinMax.MaxHeight)); */
    __area_finish_minmax(data->wd_RootObject, &data->wd_MinMax);
/*      D(bug("*** root minmax2 = %ld,%ld => %ld,%ld\n", data->wd_MinMax.MinWidth, */
/*  	  data->wd_MinMax.MinHeight, */
/*  	  data->wd_MinMax.MaxWidth, data->wd_MinMax.MaxHeight)); */
}


static void install_backbuffer (struct IClass *cl, Object *obj)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
#if 0
    struct Window *win = data->wd_RenderInfo.mri_Window;

    data->wd_RenderInfo.mri_BufferBM =
	AllocBitMap(win->Width, win->Height, win->RPort->BitMap->Depth,
		    0, win->RPort->BitMap);
#endif
    if (data->wd_RenderInfo.mri_BufferBM)
    {
/*  	D(bug("install_backbuffer : allocated bitmap %dx%dx%d with friend %p\n", */
/*  	      win->Width, win->Height, win->RPort->BitMap->Depth, win->RPort->BitMap)); */
	InitRastPort(&data->wd_RenderInfo.mri_BufferRP);
	data->wd_RenderInfo.mri_BufferRP.BitMap = data->wd_RenderInfo.mri_BufferBM;
    }
}

static void deinstall_backbuffer (struct IClass *cl, Object *obj)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    DeinitRastPort(&data->wd_RenderInfo.mri_BufferRP);
    
    if (data->wd_RenderInfo.mri_BufferBM)
    {
	FreeBitMap(data->wd_RenderInfo.mri_BufferBM);
	data->wd_RenderInfo.mri_BufferBM = NULL;
    }
}

/*
 * Called after window is opened or resized.
 * An expose event is already queued, it will trigger
 * MUIM_Draw for us when going back to main loop.
 */
static void window_Show (struct IClass *cl, Object *obj)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    struct Window *win = data->wd_RenderInfo.mri_Window;
/*      D(bug("window_show %s %d\n", __FILE__, __LINE__)); */

    _left(data->wd_RootObject) = win->BorderLeft;
    _top(data->wd_RootObject)  = win->BorderTop;
    _width(data->wd_RootObject) = data->wd_Width;
    _height(data->wd_RootObject) = data->wd_Height;

    DoMethod(data->wd_RootObject, MUIM_Layout);

    ShowRenderInfo(&data->wd_RenderInfo);
/*      D(bug("zune_imspec_show %s %d\n", __FILE__, __LINE__)); */
    zune_imspec_show(data->wd_Background, obj);
    DoMethod(data->wd_RootObject, MUIM_Show);
}

static ULONG window_Open(struct IClass *cl, Object *obj)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    if (!data->wd_RootObject)
	return FALSE;

    if (!DoMethod(obj, MUIM_Window_Setup))
	return FALSE;

    /* I got display info, so calculate your display dependant data */
    if (!DoSetupMethod(data->wd_RootObject, &data->wd_RenderInfo))
    {
	DoMethod(obj, MUIM_Window_Cleanup);
	return FALSE;
    }

    /* inquire about sizes */
    window_minmax(obj,data);
    window_select_dimensions(data);

    /* Decide which menustrip should be used */
    if (!data->wd_ChildMenustrip) get(_app(obj), MUIA_Application_Menustrip, &data->wd_Menustrip);
    else data->wd_Menustrip = data->wd_ChildMenustrip;

    /* open window here ... */
    if (!DisplayWindow(obj,data))
    {
	/* free display dependant data */
	data->wd_Menustrip = NULL;
	DoMethod(data->wd_RootObject, MUIM_Cleanup);
	DoMethod(obj, MUIM_Window_Cleanup);
	return FALSE;
    }

    install_backbuffer(cl, obj);

    data->wd_Flags |= MUIWF_OPENED;

    window_Show(cl, obj);

    {
	LONG left,top,width,height;

	left = data->wd_RenderInfo.mri_Window->BorderLeft;
	top = data->wd_RenderInfo.mri_Window->BorderTop,
	width = data->wd_RenderInfo.mri_Window->Width
	    - data->wd_RenderInfo.mri_Window->BorderRight - left;
	height = data->wd_RenderInfo.mri_Window->Height
	    - data->wd_RenderInfo.mri_Window->BorderBottom - top;

/*  	D(bug("zune_imspec_draw %s %d\n", __FILE__, __LINE__)); */
//	D(bug("%d:zune_imspec_draw(%p) l=%d t=%d w=%d h=%d xo=%d yo=%d\n",
//	      __LINE__, data->wd_Background, left, top, width,
//	      height, left, top));

	zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
		 left, top, width, height, left, top, 0);
    }

    MUI_Redraw(data->wd_RootObject, MADF_DRAWOBJECT);

    if (data->wd_OldActive)
	set(obj, MUIA_Window_ActiveObject, data->wd_OldActive);

    return TRUE;
}

/******************************************************************************/
/******************************************************************************/

static ULONG window_Close(struct IClass *cl, Object *obj)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    data->wd_OldActive = data->wd_ActiveObject;
    set(obj, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);

    /* remove from window */
    DoMethod(data->wd_RootObject, MUIM_Hide);
    zune_imspec_hide(data->wd_Background);

    deinstall_backbuffer(cl, obj);

    HideRenderInfo(&data->wd_RenderInfo);

    /* close here ... */
    UndisplayWindow(obj,data);

    data->wd_Flags &= ~MUIWF_OPENED;
    data->wd_Menustrip = NULL;

    /* free display dependant data */
    DoMethod(data->wd_RootObject, MUIM_Cleanup);
    DoMethod(obj, MUIM_Window_Cleanup);
    return TRUE;
}

/* calculate a new layout
 * see MUIA_ShowMe
 * see Group_ExitChange
 * see Group_Columns
 * see Group_Rows
 */
static ULONG Window_RecalcDisplay(struct IClass *cl, Object *obj, struct MUIP_Window_RecalcDisplay *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    LONG left,top,width,height;
    BOOL resized;

    if (!(data->wd_Flags & MUIWF_OPENED)) return 0;

    DoMethod(data->wd_RootObject, MUIM_Hide);
    deinstall_backbuffer(cl, obj);
    HideRenderInfo(&data->wd_RenderInfo);

    /* inquire about sizes */
    window_minmax(obj,data);
    /* resize window ? */
    window_select_dimensions(data);
    resized = _zune_window_resize(data);

    install_backbuffer(cl, obj);

    window_Show(cl, obj);

#if 0
    if (msg->originator && !resized)
    {
	left = _left(msg->originator);
	top = _top(msg->originator);
	width = _width(msg->originator);
	height = _height(msg->originator);

/*  	D(bug("zune_imspec_draw %s %d\n", __FILE__, __LINE__)); */
	zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
			 left, top, width, height, left, top, 0);
	MUI_Redraw(msg->originator, MADF_DRAWALL);
    }
    else
#endif
    {
	left = data->wd_RenderInfo.mri_Window->BorderLeft;
	top = data->wd_RenderInfo.mri_Window->BorderTop;
	width = data->wd_RenderInfo.mri_Window->Width
	    - data->wd_RenderInfo.mri_Window->BorderRight - left;
	height = data->wd_RenderInfo.mri_Window->Height
	    - data->wd_RenderInfo.mri_Window->BorderBottom - top;

/*  	D(bug("zune_imspec_draw %s %d : %d %d %d %d\n", __FILE__, __LINE__, */
/*  	      left, top, width, height)); */

	if (muiGlobalInfo(obj)->mgi_Prefs->window_redraw == WINDOW_REDRAW_WITHOUT_CLEAR)
	    MUI_Redraw(data->wd_RootObject, MADF_DRAWOBJECT);
	else
	{
//	    D(bug("%d:zune_imspec_draw(%p) l=%d t=%d w=%d h=%d xo=%d yo=%d\n",
//		  __LINE__, data->wd_Background, left, top, width,
//		  height, left, top));
	    zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
			     left, top, width, height, left, top, 0);
	    MUI_Redraw(data->wd_RootObject, MADF_DRAWALL);
	}
    }
    return TRUE;
}


/**************************************************************************
 ...
**************************************************************************/
static ULONG Window_AddEventHandler(struct IClass *cl, Object *obj,
                 struct MUIP_Window_AddEventHandler *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

//    D(bug("muimaster.library/window.c: Add Eventhandler\n"));

#ifdef __AROS__
    msg->ehnode->ehn_Node.ln_Pri = msg->ehnode->ehn_Priority;
#endif
    Enqueue((struct List *)&data->wd_EHList, (struct Node *)msg->ehnode);
    _zune_window_change_events(data, _zune_window_get_default_events());
    return TRUE;
}

/**************************************************************************
 ...
**************************************************************************/
static ULONG Window_RemEventHandler(struct IClass *cl, Object *obj,
                 struct MUIP_Window_RemEventHandler *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

//    D(bug("muimaster.library/window.c: Rem Eventhandler\n"));

    Remove((struct Node *)msg->ehnode);
    _zune_window_change_events(data, _zune_window_get_default_events());
    return TRUE;
}

/**************************************************************************
 Note that this is MUIM_Window_Setup, not MUIM_Setup
**************************************************************************/
static ULONG Window_Setup(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    if (!SetupRenderInfo(obj, data, &data->wd_RenderInfo))
	return FALSE;

    data->wd_Background = zune_imspec_setup(MUII_WindowBack, &data->wd_RenderInfo);

    if (muiGlobalInfo(obj)->mgi_Prefs->window_redraw == WINDOW_REDRAW_WITH_CLEAR)
	data->wd_Flags |= MUIWF_ERASEAREA;

    return TRUE;
}

/**************************************************************************

**************************************************************************/
static ULONG Window_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    zune_imspec_cleanup(data->wd_Background);

    if (data->wd_dnd)
    {
    	DeleteDragNDrop(data->wd_dnd);
    	data->wd_dnd = NULL;
    }

    CleanupRenderInfo(&data->wd_RenderInfo);
    return TRUE;
}


/**************************************************************************
 This adds the the control char handler and also do the MUIA_CycleChain
 stuff. Orginal MUI does this in an other way.
**************************************************************************/
static ULONG Window_AddControlCharHandler(struct IClass *cl, Object *obj, struct MUIP_Window_AddControlCharHandler *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    if (msg->ccnode->ehn_Events)
    {
#ifdef __AROS__
	msg->ccnode->ehn_Node.ln_Pri = msg->ccnode->ehn_Priority;
#endif
	Enqueue((struct List *)&data->wd_CCList, (struct Node *)msg->ccnode);
    }
    /* Due to the lack of an better idea ... */
    if (muiAreaData(msg->ccnode->ehn_Object)->mad_Flags & MADF_CYCLECHAIN)
    {
	struct ObjNode *node = mui_alloc_struct(struct ObjNode);
	if (node)
	{
	    node->obj = msg->ccnode->ehn_Object;
	    AddTail((struct List *)&data->wd_CycleChain,(struct Node*)node);
	}
    }
    return TRUE;
}

/**************************************************************************

**************************************************************************/
static ULONG Window_RemControlCharHandler(struct IClass *cl, Object *obj, struct MUIP_Window_RemControlCharHandler *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    struct ObjNode     *node = FindObjNode(&data->wd_CycleChain,msg->ccnode->ehn_Object);

    if (msg->ccnode->ehn_Events) Remove((struct Node *)msg->ccnode);

    if (node)
    {
    	/* Remove from the chain list */
	Remove((struct Node *)node);
	mui_free(node);
    }

    return TRUE;
}

/**************************************************************************

**************************************************************************/
static ULONG Window_DragObject(struct IClass *cl, Object *obj, struct MUIP_Window_DragObject *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (msg->obj)
    {
	struct DragNDrop *dnd;
	struct MUI_DragImage *di;
	struct BitMapNode *bmn;

	if (!(dnd = CreateDragNDropA(NULL))) return NULL;
	if (!(di = (struct MUI_DragImage*)DoMethod(msg->obj,MUIM_CreateDragImage,-msg->touchx,-msg->touchy,msg->flags)))
	{
	    DeleteDragNDrop(dnd);
	    return 0;
	}
	if (!di->bm)
	{
	    DoMethod(msg->obj,MUIM_DeleteDragImage, (IPTR)di);
	    DeleteDragNDrop(dnd);
	    return 0;
	}

	if (!(bmn = CreateBitMapNode(
		GUI_BitMap, di->bm,
		GUI_LeftOffset, di->touchx,
		GUI_TopOffset, di->touchy,
		GUI_Width, di->width,
		GUI_Height, di->height,
		TAG_DONE)))
	{
	    DoMethod(msg->obj,MUIM_DeleteDragImage, (IPTR)di);
	    DeleteDragNDrop(dnd);
	    return 0;
	}

	AttachBitMapNode(dnd,bmn);

	if (!PrepareDragNDrop(dnd, data->wd_RenderInfo.mri_Screen))
	{
	    DoMethod(msg->obj,MUIM_DeleteDragImage, (IPTR)di);
	    DeleteDragNDrop(dnd);
	    return 0;
	}

	muiAreaData(msg->obj)->mad_Flags |= MADF_DRAGGING;

	data->wd_DragObject = msg->obj;
	data->wd_dnd = dnd;
	data->wd_DragImage = di;
	return 1;
    }
    return 0;
}

/**************************************************************************

**************************************************************************/
static IPTR Window_AllocGadgetID(struct IClass *cl, Object *obj, struct MUIP_Window_AllocGadgetID *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    struct IDNode *newnode = mui_alloc_struct(struct IDNode);

    if (newnode)
    {
	int id;
	struct MinNode *mn;

	if (IsListEmpty((struct List*)&data->wd_IDList))
	{
	    newnode->id = 1;
	    AddHead((struct List*)&data->wd_IDList,(struct Node*)&newnode->node);
	    return (IPTR)1;
	}

	id = 1;

	for (mn = data->wd_IDList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
	{
	    struct IDNode *idn = (struct IDNode *)mn;
	    if (id < idn->id) break;
	    id++;
	}
	newnode->id = id;
	Insert((struct List*)&data->wd_IDList,(struct Node*)&newnode->node,(struct Node*)mn);
	return (IPTR)id;
    }
    return 0;
}

/**************************************************************************

**************************************************************************/
static IPTR Window_FreeGadgetID(struct IClass *cl, Object *obj, struct MUIP_Window_FreeGadgetID *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    struct MinNode *mn;

    for (mn = data->wd_IDList.mlh_Head; mn->mln_Succ; mn = mn->mln_Succ)
    {
	struct IDNode *idn = (struct IDNode *)mn;
	if (msg->gadgetid == idn->id)
	{
	    Remove((struct Node*)idn);
	    mui_free(idn);
	    return 0;
	}
    }

    return 0;
}


/**************************************************************************
 MUIM_Window_GetMenuCheck
**************************************************************************/
static IPTR Window_GetMenuCheck(struct IClass *cl, Object *obj, struct MUIP_Window_GetMenuCheck *msg)
{
    IPTR stat;
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    Object *item;
    Object *strip = data->wd_ChildMenustrip;
    if (!strip) strip = data->wd_Menustrip;
    if (!strip) return 0;
    if (!(item = (Object*)DoMethod(strip, MUIM_FindUData, msg->MenuID))) return 0;
    get(item,MUIA_Menuitem_Checked, &stat);
    return stat;
}

/**************************************************************************
 MUIM_Window_SetMenuCheck
**************************************************************************/
static IPTR Window_SetMenuCheck(struct IClass *cl, Object *obj, struct MUIP_Window_SetMenuCheck *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    Object *item;
    Object *strip = data->wd_ChildMenustrip;
    if (!strip) strip = data->wd_Menustrip;
    if (!strip) return 0;
    if (!(item = (Object*)DoMethod(strip, MUIM_FindUData, msg->MenuID))) return 0;
    set(item,MUIA_Menuitem_Checked,msg->stat);
    return 0;
}

#if 0 /* unused? */

/**************************************************************************
 MUIM_Window_GetMenuState
**************************************************************************/
static IPTR Window_GetMenuState(struct IClass *cl, Object *obj, struct MUIP_Window_GetMenuState *msg)
{
    IPTR stat;
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    Object *item;
    Object *strip = data->wd_ChildMenustrip;
    if (!strip) strip = data->wd_Menustrip;
    if (!strip) return 0;
    if (!(item = (Object*)DoMethod(strip, MUIM_FindUData, msg->MenuID))) return 0;
    get(item,MUIA_Menuitem_Enabled, &stat);
    return stat;
}

/**************************************************************************
 MUIM_Window_SetMenuState
**************************************************************************/
static IPTR Window_SetMenuState(struct IClass *cl, Object *obj, struct MUIP_Window_SetMenuState *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    Object *item;
    Object *strip = data->wd_ChildMenustrip;
    if (!strip) strip = data->wd_Menustrip;
    if (!strip) return 0;
    if (!(item = (Object*)DoMethod(strip, MUIM_FindUData, msg->MenuID))) return 0;
    set(item,MUIA_Menuitem_Enabled,msg->stat);
    return 0;
}

#endif /* unused? */

/**************************************************************************
 MUIM_Window_DrawBackground
**************************************************************************/
static IPTR Window_DrawBackground(struct IClass *cl, Object *obj, struct MUIP_Window_DrawBackground *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (!(data->wd_RenderInfo.mri_Window)) /* not between show/hide */
	return FALSE;

//    D(bug("%d:zune_imspec_draw(%p) l=%d t=%d w=%d h=%d xo=%d yo=%d\n",
//	  __LINE__, data->wd_Background, msg->left, msg->top, msg->width,
//	  msg->height, msg->xoffset, msg->yoffset));
    zune_imspec_draw(data->wd_Background, &data->wd_RenderInfo,
		    msg->left, msg->top, msg->width, msg->height,
		    msg->xoffset, msg->yoffset, 0);
    return 0;
}

/**************************************************************************
 MUIM_Window_ToFront
**************************************************************************/
static IPTR Window_ToFront(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (!(data->wd_RenderInfo.mri_Window)) /* not between show/hide */
	return 0;

    WindowToFront(data->wd_RenderInfo.mri_Window);
    return 1;
}

/**************************************************************************
 MUIM_Window_ToBack
**************************************************************************/
static IPTR Window_ToBack(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (!(data->wd_RenderInfo.mri_Window)) /* not between show/hide */
	return 0;

    WindowToBack(data->wd_RenderInfo.mri_Window);
    return 1;
}

/**************************************************************************
 MUIM_Window_ScreenToBack
**************************************************************************/
static IPTR Window_ScreenToBack(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (!(data->wd_RenderInfo.mri_Window)) /* not between show/hide */
	return 0;

    ScreenToBack(data->wd_RenderInfo.mri_Screen);
    return 1;
}

/**************************************************************************
 MUIM_Window_ScreenToFront
**************************************************************************/
static IPTR Window_ScreenToFront(struct IClass *cl, Object *obj, Msg msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);
    if (!(data->wd_RenderInfo.mri_Window)) /* not between show/hide */
	return 0;

    ScreenToFront(data->wd_RenderInfo.mri_Screen);
    return 1;
}

/**************************************************************************
 MUIM_Window_ActionIconify
**************************************************************************/
static IPTR Window_ActionIconify(struct IClass *cl, Object *obj, Msg msg)
{
    set(_app(obj),MUIA_Application_Iconified,TRUE);
    return 1;
}


/* Loads ENV: prefs, add a Window_ID chunk in the MUIW chunk, if no MUIW chunk
 * then create it at the same level as MUIC chunk, save prefs.
 * Do the same for ENVARC:
 * MUIW chunk layout:
 * 'MUIW'
 * 00 00 00 30 (chunk length for a single window, 0x30L big endian)
 * 'this window ID'
 * 00 00 00 28
 * xx xx yy yy  (X, Y)
 * ww ww hh hh  (Width, Height)
 * ax ax ay ay  (AltX, AltY)
 * aw aw ah ah  (AltWidth, AltHeight)
 * 00 00 00 00  (???)
 * 00 00 00 00
 * 00 00 00 00
 * 00 00 00 00
 * 00 01 00 00
 * 00 00 00 00
 */
static void RememberWindowPosition(Object *winobj, ULONG id)
{
    if (!id)
	return;
}

/* Loads ENV: prefs, remove our Window_ID chunk from the MUIW chunk, save prefs.
 * Do the same for ENVARC:
 * This function shouldnt really be in window.c, but rather in a file dealing
 * with prefs file stuff.
 */
static void ForgetWindowPosition(Object *winobj, ULONG id)
{
    if (!id)
	return;
}

/**************************************************************************
 MUIM_Window_Snapshot
**************************************************************************/
static IPTR Window_Snapshot(struct IClass *cl, Object *obj, struct MUIP_Window_Snapshot *msg)
{
    struct MUI_WindowData *data = INST_DATA(cl, obj);

    if (msg->flags)
	RememberWindowPosition(obj, data->wd_ID);
    else
	ForgetWindowPosition(obj, data->wd_ID);
    return 1;
}



BOOPSI_DISPATCHER(IPTR, Window_Dispatcher, cl, obj, msg)
{
    switch (msg->MethodID)
    {
	/* Whenever an object shall be created using NewObject(), it will be
	** sent a OM_NEW method.
	*/
	case OM_NEW: return Window_New(cl, obj, (struct opSet *) msg);
	case OM_DISPOSE: return Window_Dispose(cl, obj, msg);
	case OM_SET: return Window_Set(cl, obj, (struct opSet *)msg);
	case OM_GET: return Window_Get(cl, obj, (struct opGet *)msg);
	case MUIM_Window_AddEventHandler: return Window_AddEventHandler(cl, obj, (APTR)msg);
	case MUIM_Window_RemEventHandler: return Window_RemEventHandler(cl, obj, (APTR)msg);
	case MUIM_ConnectParent: return Window_ConnectParent(cl, obj, (APTR)msg);
	case MUIM_DisconnectParent: return Window_DisconnectParent(cl, obj, (APTR)msg);
	case MUIM_Window_RecalcDisplay: return Window_RecalcDisplay(cl, obj, (APTR)msg);
	case MUIM_Window_Setup: return Window_Setup(cl, obj, (APTR)msg);
	case MUIM_Window_Cleanup: return Window_Cleanup(cl, obj, (APTR)msg);
	case MUIM_Window_AddControlCharHandler: return Window_AddControlCharHandler(cl, obj, (APTR)msg);
	case MUIM_Window_RemControlCharHandler: return Window_RemControlCharHandler(cl, obj, (APTR)msg);
	case MUIM_Window_DragObject: return Window_DragObject(cl, obj, (APTR)msg);
	case MUIM_Window_AllocGadgetID: return Window_AllocGadgetID(cl, obj, (APTR)msg);
	case MUIM_Window_FreeGadgetID: return Window_FreeGadgetID(cl, obj, (APTR)msg);
	case MUIM_Window_GetMenuCheck: return Window_GetMenuCheck(cl, obj, (APTR)msg);
	case MUIM_Window_SetMenuCheck: return Window_SetMenuCheck(cl, obj, (APTR)msg);
	case MUIM_Window_GetMenuState: return Window_GetMenuCheck(cl, obj, (APTR)msg);
	case MUIM_Window_SetMenuState: return Window_SetMenuCheck(cl, obj, (APTR)msg);
	case MUIM_Window_DrawBackground: return Window_DrawBackground(cl, obj, (APTR)msg);
	case MUIM_Window_ToFront: return Window_ToFront(cl, obj, (APTR)msg);
	case MUIM_Window_ToBack: return Window_ToBack(cl, obj, (APTR)msg);
	case MUIM_Window_ScreenToFront: return Window_ScreenToFront(cl, obj, (APTR)msg);
	case MUIM_Window_ScreenToBack: return Window_ScreenToBack(cl, obj, (APTR)msg);
	case MUIM_Window_ActionIconify: return Window_ActionIconify(cl, obj, (APTR)msg);
	case MUIM_Window_Snapshot: return Window_Snapshot(cl, obj, (APTR)msg);
    }

    return DoSuperMethodA(cl, obj, msg);
}


/*
 * Class descriptor.
 */
const struct __MUIBuiltinClass _MUI_Window_desc = {
    MUIC_Window,
    MUIC_Notify,
    sizeof(struct MUI_WindowData),
    (void*)Window_Dispatcher
};

