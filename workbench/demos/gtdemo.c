/*
   (C) 1997-98 AROS - The Amiga Replacement OS
   $Id$

   Desc: Demo for gadtools.library
   Lang: english
 */

#include <aros/config.h>

#if !(AROS_FLAVOUR & AROS_FLAVOUR_NATIVE)
#   define ENABLE_RT 1
#endif

#include <stdio.h>
#include <proto/exec.h>
#include <exec/libraries.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <proto/intuition.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <utility/tagitem.h>
#include <proto/gadtools.h>
#include <libraries/gadtools.h>

#include <proto/alib.h>

#include <aros/rt.h>

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>
struct IntuitionBase *IntuitionBase;
struct Library *GadToolsBase;

APTR vi;
struct Screen *scr;
struct Window *win;
struct Gadget *glist = NULL;

struct Gadget *button;

#define ID_BUTTON 1
#define ID_CHECKBOX 2
#define ID_MX 3
#define ID_CYCLE 4
#define ID_PALETTE 5
#define ID_SLIDER 6
#define ID_SCROLLER 7
#define ID_STRING 8
#define ID_INTEGER 9

struct NewGadget buttongad =
{
    210, 10, 100, 20,
    "Exit (1)", NULL,
    ID_BUTTON, PLACETEXT_IN, NULL, NULL
};

struct NewGadget checkbox =
{
    320, 10, 20, 20,
    "Disable (2)", NULL,
    ID_CHECKBOX, PLACETEXT_RIGHT, NULL, NULL
};

struct NewGadget cyclegad =
{
    210, 140, 100, 20,
    NULL, NULL,
    ID_CYCLE, 0, NULL, NULL
};

struct NewGadget mxgad =
{
    210, 60, MX_WIDTH, 20,
    "Mutual Exclude (3)", NULL,
    ID_MX, PLACETEXT_RIGHT, NULL, NULL
};

struct NewGadget palettegad =
{
    210, 180, 120, 100,
    "Palette (5)", NULL,
    ID_PALETTE, PLACETEXT_ABOVE, NULL, NULL
};

struct NewGadget textgad =
{
    380, 40, 120, 30,
    NULL, NULL,
    0, 0, NULL, NULL
};

struct NewGadget numbergad =
{
    380, 80, 70, 20,
    NULL, NULL,
    0, 0, NULL, NULL
};

struct NewGadget slidergad =
{
    380, 130, 120, 20,
    "Slider (6)", NULL,
    ID_SLIDER, PLACETEXT_ABOVE, NULL, NULL
};

struct NewGadget scrollergad =
{
    380, 160, 20, 100,
    NULL, NULL,
    ID_SCROLLER, 0, NULL, NULL
};

struct NewGadget stringgad =
{
    420, 180, 100, 20,
    "String (8)", NULL,
    ID_STRING, PLACETEXT_ABOVE, NULL, NULL
};

struct NewGadget integergad =
{
    420, 240, 60, 20,
    "Integer (9)", NULL,
    ID_INTEGER, PLACETEXT_ABOVE, NULL, NULL
};

STRPTR mxlabels[] =
{
    "Label 1",
    "Label 2",
    "Label 3",
    NULL
};

STRPTR cyclelabels[] =
{
    "Label 1",
    "Label 2",
    "Label 3",
    NULL
};


BOOL openlibs()
{
    IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 37);
    GadToolsBase = OpenLibrary("gadtools.library", 0);
    if (!IntuitionBase) {
	printf("GTDemo: Error opening intuition.library\n");
	return FALSE;
    }
    if (!GadToolsBase) {
        printf("GTDemo: Error opening gadtools.library\n");
	return FALSE;
    }
    return TRUE;
}

void closelibs()
{
D(bug("Closelibs: closing gadtools\n"));
    CloseLibrary(GadToolsBase);
D(bug("Closelibs: closing intuition\n"));
    CloseLibrary((struct Library *) IntuitionBase);
D(bug("Closelibs: libs closed\n"));
}


struct Gadget *gt_init()
{
    struct Gadget *gad = NULL;

    scr = LockPubScreen(NULL);
    vi = GetVisualInfoA(scr, NULL);
    if (vi != NULL)
	gad = CreateContext(&glist);
    return gad;
}

void gt_end()
{
    D(bug("gtend: Freeing gadgets\n"));
    FreeGadgets(glist);
    D(bug("gtend: Freeing visualnfo\n"));
    FreeVisualInfo(vi);
    D(bug("gtend: Unlocking screen\n"));
    UnlockPubScreen(NULL, scr);
}


BOOL openwin()
{
    win = OpenWindowTags(NULL,
			 WA_PubScreen, scr,
			 WA_Left, 0,
			 WA_Top, 0,
			 WA_Width, 600,
			 WA_Height, 300,
			 WA_Title, "GTDemo",
			 WA_IDCMP,
			     BUTTONIDCMP |
			     CHECKBOXIDCMP |
                             CYCLEIDCMP |
                             MXIDCMP |
                             PALETTEIDCMP |
                             SLIDERIDCMP |
                             IDCMP_GADGETUP |
			     IDCMP_VANILLAKEY |
			     IDCMP_CLOSEWINDOW |
			     IDCMP_REFRESHWINDOW,
			 WA_SimpleRefresh, TRUE,
			 WA_Gadgets, glist,
			 WA_DragBar, TRUE,
			 WA_CloseGadget, TRUE,
			 TAG_DONE);
    if (!win) {
	printf("GTDemo: Error opening window\n");
	return FALSE;
    }
    return TRUE;
}


struct Gadget *makegadgets(struct Gadget *gad)
{
    buttongad.ng_VisualInfo = vi;
    checkbox.ng_VisualInfo = vi;
    cyclegad.ng_VisualInfo = vi;
    mxgad.ng_VisualInfo = vi;
    palettegad.ng_VisualInfo = vi;
    textgad.ng_VisualInfo = vi;
    numbergad.ng_VisualInfo = vi;
    slidergad.ng_VisualInfo = vi;
    scrollergad.ng_VisualInfo = vi;
    stringgad.ng_VisualInfo = vi;
    integergad.ng_VisualInfo = vi;

    gad = CreateGadget(BUTTON_KIND, gad, &buttongad,
                       GA_Immediate, TRUE,
                       TAG_DONE);
D(bug("Created button gadget: %p\n", gad));
    button = gad;
    gad = CreateGadget(CHECKBOX_KIND, gad, &checkbox,
                       GTCB_Checked, FALSE,
                       GTCB_Scaled, TRUE,
                       TAG_DONE);
D(bug("Created checkbox gadget: %p\n", gad));
    gad = CreateGadget(CYCLE_KIND, gad, &cyclegad,
                       GTCY_Labels, &cyclelabels,
                       TAG_DONE);
D(bug("Created cycle gadget: %p\n", gad));
    gad = CreateGadget(MX_KIND, gad, &mxgad,
		       GTMX_Labels, &mxlabels,
                       GTMX_Scaled, TRUE,
                       GTMX_TitlePlace, PLACETEXT_ABOVE,
		       TAG_DONE);

D(bug("Created mx gadget: %p\n", gad));
    gad = CreateGadget(PALETTE_KIND, gad, &palettegad,
    		       GTPA_NumColors,		6,
    		       GTPA_IndicatorHeight,	30,
    		       GTPA_Color,		0,
		       TAG_DONE);

D(bug("Created palette gadget: %p\n", gad));
    gad = CreateGadget(TEXT_KIND, gad, &textgad,
    		       GTTX_Text,	"Text display",
    		       GTTX_CopyText,	TRUE,
    		       GTTX_Border,	TRUE,
    		       GTTX_Justification,	GTJ_CENTER,
		       TAG_DONE);

D(bug("Created text gadget: %p\n", gad));
    gad = CreateGadget(NUMBER_KIND, gad, &numbergad,
    		       GTNM_Number,	10,
    		       GTNM_Border,	TRUE,
    		       GTNM_Justification,	GTJ_CENTER,
		       TAG_DONE);
    
D(bug("Created number gadget: %p\n", gad));
    gad = CreateGadget(SLIDER_KIND, gad, &slidergad,
    		       GTSL_Min,		10,
    		       GTSL_Max,		20,
    		       GTSL_Level,		12,
    		       GTSL_MaxLevelLen,	3,
    		       GTSL_LevelFormat,	"%2ld",
    		       GTSL_LevelPlace,		PLACETEXT_RIGHT,
    		       GTSL_Justification,	GTJ_RIGHT,
    		       PGA_Freedom,		LORIENT_HORIZ,
		       TAG_DONE);


D(bug("Created slider gadget: %p\n", gad));
    gad = CreateGadget(SCROLLER_KIND, gad, &scrollergad,
    		       GTSC_Top,		2,
    		       GTSC_Total,		10,
    		       GTSC_Visible,		2,
    		       GTSC_Arrows,		10,
    		       GA_RelVerify,		TRUE,
    		       PGA_Freedom,		LORIENT_VERT,
		       TAG_DONE);

D(bug("Created scroller gadget: %p\n", gad));
    gad = CreateGadget(STRING_KIND, gad, &stringgad,
    		       GTST_String,		"Blahblahblah",
    		       GTST_MaxChars,		80,
    		       GTSC_Visible,		2,
    		       GA_Immediate,		TRUE,
		       TAG_DONE);

D(bug("Created string gadget: %p\n", gad));

    gad = CreateGadget(INTEGER_KIND, gad, &integergad,
    		       GTIN_Number,		100,
    		       GTIN_MaxChars,		5,
    		       STRINGA_Justification,	GACT_STRINGCENTER,
    		       GA_Immediate,		TRUE,
		       TAG_DONE);

D(bug("Created integer gadget: %p\n", gad));

    if (!gad) {
        FreeGadgets(glist);
        printf("GTDemo: Error creating gadgets\n");
    }
    return gad;
}

void draw_bevels(struct Window *win, APTR vi)
{
    DrawBevelBox(win->RPort, 10, 10, 80, 80,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
    DrawBevelBox(win->RPort, 110, 10, 80, 80,
                 GTBB_Recessed, TRUE,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
    DrawBevelBox(win->RPort, 10, 110, 80, 80,
                 GTBB_FrameType, BBFT_RIDGE,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
    DrawBevelBox(win->RPort, 110, 110, 80, 80,
                 GTBB_FrameType, BBFT_RIDGE, GTBB_Recessed, TRUE,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
    DrawBevelBox(win->RPort, 10, 210, 80, 80,
                 GTBB_FrameType, BBFT_ICONDROPBOX,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
    DrawBevelBox(win->RPort, 110, 210, 80, 80,
                 GTBB_FrameType, BBFT_ICONDROPBOX, GTBB_Recessed, TRUE,
                 GT_VisualInfo, (IPTR) vi, TAG_DONE);
}

void handlewin()
{
    BOOL ready = FALSE;
    struct IntuiMessage *msg;

    while (ready == FALSE) {
	WaitPort(win->UserPort);
	msg = GT_GetIMsg(win->UserPort);
	if (msg != NULL) {
	    switch (msg->Class) {
	    case IDCMP_REFRESHWINDOW:
		GT_BeginRefresh(win);
		draw_bevels(win, vi);
		GT_EndRefresh(win, TRUE);
		break;
            case IDCMP_VANILLAKEY:
                if (msg->Code != 0x1B) /* if escape, quit */
                    break;
	    case IDCMP_CLOSEWINDOW:
		ready = TRUE;
		break;
	    case IDCMP_GADGETDOWN:
		printf("Gadget %d pressed",
		       ((struct Gadget *) msg->IAddress)->GadgetID);
                switch (((struct Gadget *) msg->IAddress)->GadgetID) {
                case ID_MX:
                    printf(" (active: %d)", msg->Code);
                    break;
                }
                printf("\n");
                break;
            case IDCMP_MOUSEMOVE:
            	if (msg->IAddress)
            	{
            	    switch (((struct Gadget *) msg->IAddress)->GadgetID) {
            	    case ID_SLIDER:
            	    	printf("Slider moved to value %d\n", msg->Code);
            	    	break;
            	    	
            	    case ID_SCROLLER:
            	    	printf("Scroller moved to value %d\n", msg->Code);
            	    	break;

            	    }

            	}
            	break;
            	
	    case IDCMP_GADGETUP:
		printf("Gadget %d released",
		       ((struct Gadget *) msg->IAddress)->GadgetID);
		switch (((struct Gadget *) msg->IAddress)->GadgetID) {
		case ID_BUTTON:
		    ready = TRUE;
		    break;
		    
		case ID_PALETTE:
		    printf(" (color: %d)", msg->Code);
		    break;
		    
		case ID_CHECKBOX:{
                    BOOL checked;

                    checked = msg->Code;
                    if (checked)
                        printf(" (checked)");
                    else
                        printf(" (not checked)");
                    GT_SetGadgetAttrs(button, win, NULL,
                                      GA_Disabled, (IPTR)checked, TAG_DONE);
                    break;
                }
		}
		printf("\n");
		break;
	    }
	    GT_ReplyIMsg(msg);
	}
    }
}


int main()
{
    int error = RETURN_OK;



#if SDEBUG     /* Debugging hack */
    struct Task *idtask;
    SDInit();
    if ((idtask = FindTask("input.device")))
    	idtask->tc_UserData = NULL;
#endif    	

    RT_Init();

    if (openlibs() != FALSE) {
	struct Gadget *gad;

	gad = gt_init();
	gad = makegadgets(gad);
	if (gad != NULL) {
	    if (openwin() != FALSE) {
		draw_bevels(win, vi);
		handlewin();
		CloseWindow(win);
	    } else
		error = RETURN_FAIL;
	} else
	    error = RETURN_FAIL;
	    
	D(bug("Doing gt_end()\n"));
	gt_end();
    } else
	error = RETURN_FAIL;

D(bug("closing libs\n"));
    closelibs();

    RT_Exit();

    return (error);
}
