/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: English
*/

/*********************************************************************************************/

#include "global.h"
#include "registertab.h"

#include <stdlib.h> /* for exit() */
#include <stdio.h>
#include <string.h>

/*********************************************************************************************/

#define ARG_TEMPLATE    "FROM,EDIT/S,USE/S,SAVE/S,MAP/K,PUBSCREEN/K"

#define ARG_FROM        0
#define ARG_EDIT    	1
#define ARG_USE     	2
#define ARG_SAVE      	3
#define ARG_MAP     	4
#define ARG_PUBSCREEN   5

#define NUM_ARGS        6

/*********************************************************************************************/

static struct libinfo
{
    APTR        var;
    STRPTR      name;
    WORD        version;
    BOOL    	required;
}
libtable[] =
{
    {&IntuitionBase     , "intuition.library"	 , 39, TRUE  },
    {&GfxBase           , "graphics.library" 	 , 39, TRUE  },
    {&GadToolsBase      , "gadtools.library" 	 , 39, TRUE  },
    {&UtilityBase       , "utility.library"  	 , 39, TRUE  },
    {&IFFParseBase      , "iffparse.library" 	 , 39, TRUE  },
    {&CyberGfxBase  	, "cybergraphics.library", 39, FALSE },
    {NULL                                            	     }
};

#define NUM_PAGES 3

static struct page
{
    LONG nameid;
    LONG (*handler)(LONG, IPTR);
    LONG minw;
    LONG minh;
}
pagetable[NUM_PAGES] =
{
    {MSG_GAD_TAB_LANGUAGE, page_language_handler},
    {MSG_GAD_TAB_COUNTRY , page_country_handler },
    {MSG_GAD_TAB_TIMEZONE, page_timezone_handler}
};

/*********************************************************************************************/

static struct RegisterTabItem 	regitems[NUM_PAGES + 1];
static struct RegisterTab   	reg;
static struct RDArgs        	*myargs;
static WORD 	    	    	activetab;
static IPTR                 	args[NUM_ARGS];

/*********************************************************************************************/

static void CloseLibs(void);
static void FreeArguments(void);
static void FreeVisual(void);
static void KillPages(void);
static void KillWin(void);

/*********************************************************************************************/

WORD ShowMessage(STRPTR title, STRPTR text, STRPTR gadtext)
{
    struct EasyStruct es;
    
    es.es_StructSize   = sizeof(es);
    es.es_Flags        = 0;
    es.es_Title        = title;
    es.es_TextFormat   = text;
    es.es_GadgetFormat = gadtext;
   
    return EasyRequestArgs(win, &es, NULL, NULL);  
}

/*********************************************************************************************/

void Cleanup(STRPTR msg)
{
    if (msg)
    {
	if (IntuitionBase && !((struct Process *)FindTask(NULL))->pr_CLI)
	{
	    ShowMessage("Locale", msg, MSG(MSG_OK));     
	} else {
	    printf("Locale: %s\n", msg);
	}
    }
    
    KillWin();
    KillPages();
    KillMenus();
    FreeVisual();
    FreeArguments();
    CloseLibs();
    CleanupLocale();
    
    exit(prog_exitcode);
}


/*********************************************************************************************/

static void OpenLibs(void)
{
    struct libinfo *li;
    
    for(li = libtable; li->var; li++)
    {
	if (!((*(struct Library **)li->var) = OpenLibrary(li->name, li->version)))
	{
	    if (li->required)
	    {
	    	sprintf(s, MSG(MSG_CANT_OPEN_LIB), li->name, li->version);
	    	Cleanup(s);
	    }
	}       
    }
       
}

/*********************************************************************************************/

static void CloseLibs(void)
{
    struct libinfo *li;
    
    for(li = libtable; li->var; li++)
    {
	if (*(struct Library **)li->var) CloseLibrary((*(struct Library **)li->var));
    }
}

/*********************************************************************************************/

static void GetArguments(void)
{
    if (!(myargs = ReadArgs(ARG_TEMPLATE, args, NULL)))
    {
	Fault(IoErr(), 0, s, 256);
	Cleanup(s);
    }
}

/*********************************************************************************************/

static void FreeArguments(void)
{
    if (myargs) FreeArgs(myargs);
}

/*********************************************************************************************/

static void GetVisual(void)
{
    scr = LockPubScreen((CONST_STRPTR)args[ARG_PUBSCREEN]);
    if (!scr) Cleanup(MSG(MSG_CANT_LOCK_SCR));
    
    dri = GetScreenDrawInfo(scr);
    if (!dri) Cleanup(MSG(MSG_CANT_GET_DRI));
    
    vi = GetVisualInfoA(scr, NULL);
    if (!vi) Cleanup(MSG(MSG_CANT_GET_VI));
}

/*********************************************************************************************/

static void FreeVisual(void)
{
    if (dri) FreeScreenDrawInfo(scr, dri);
    if (scr) UnlockPubScreen(NULL, scr);
}

/*********************************************************************************************/

static void MakePages(void)
{
    WORD i;
    
    for(i = 0; i < NUM_PAGES; i++)
    {
    	regitems[i].text = MSG(pagetable[i].nameid);
	
	if (!(pagetable[i].handler(PAGECMD_INIT, 0)))
	{
	    Cleanup(MSG(MSG_CANT_CREATE_GADGET));
	}
    }
    
    InitRegisterTab(&reg, regitems);
    
}

/*********************************************************************************************/

static void KillPages(void)
{
    WORD i;
    
    for(i = 0; i < NUM_PAGES; i++)
    {
 	pagetable[i].handler(PAGECMD_CLEANUP, 0);
    }
}

/*********************************************************************************************/

static void LayoutGUI(void)
{
    WORD x, w, max_pagewidth = 0;
    WORD y, h, max_pageheight = 0;
    WORD i;
    
    for(i = 0; i < NUM_PAGES; i++)
    {
    	if (!(pagetable[i].handler(PAGECMD_LAYOUT, 0)))
	    Cleanup(MSG(MSG_CANT_CREATE_GADGET));
	    
	w = pagetable[i].handler(PAGECMD_GETMINWIDTH, 0);
	h = pagetable[i].handler(PAGECMD_GETMINHEIGHT, 0);
	
	if (w > max_pagewidth)  max_pagewidth  = w;
	if (h > max_pageheight) max_pageheight = h;
    }
    
    LayoutRegisterTab(&reg, scr, dri, TRUE);
    if (reg.width > max_pagewidth) max_pagewidth = reg.width;
    
    SetRegisterTabPos(&reg, scr->WBorLeft + BORDER_X, scr->WBorTop + scr->Font->ta_YSize + 1 + BORDER_Y);
    
    pages_left   = scr->WBorLeft + BORDER_X + TABBORDER_X ;
    pages_top    = scr->WBorTop + scr->Font->ta_YSize + 1 + BORDER_Y + reg.height + TABBORDER_Y;
    pages_width  = max_pagewidth;
    pages_height = max_pageheight;
    
    SetRegisterTabFrameSize(&reg, pages_width  + TABBORDER_X * 2,
    	    	    	    	  pages_height + TABBORDER_Y * 2);
    
    for(i = 0; i < NUM_PAGES; i++)
    {
    	pagetable[i].handler(PAGECMD_SETDOMLEFT  , pages_left    );
	pagetable[i].handler(PAGECMD_SETDOMTOP 	 , pages_top     );
    	pagetable[i].handler(PAGECMD_SETDOMWIDTH , max_pagewidth );
	pagetable[i].handler(PAGECMD_SETDOMHEIGHT, max_pageheight);
    }
    
    winwidth  = pages_width + TABBORDER_X * 2 + BORDER_X * 2;
    winheight = pages_height + reg.height + TABBORDER_Y * 2 + BORDER_Y * 2;
}

/*********************************************************************************************/

static void MakeWin(void)
{
    WORD wx, wy;
    
    wx = (scr->Width - (winwidth + scr->WBorLeft + scr->WBorTop)) / 2;
    wy = (scr->Height - (winheight + scr->WBorTop + scr->Font->ta_YSize + 1 + scr->WBorBottom)) / 2;
    
    win = OpenWindowTags(0, WA_PubScreen, (IPTR)scr,
    	    	    	    WA_Left, wx,
			    WA_Top, wy,
			    WA_InnerWidth, winwidth,
			    WA_InnerHeight, winheight,
			    WA_Title, (IPTR)MSG(MSG_WINTITLE),
			    WA_CloseGadget, TRUE,
			    WA_DragBar, TRUE,
			    WA_DepthGadget, TRUE,
			    WA_Activate, TRUE,
			    WA_IDCMP, REGISTERTAB_IDCMP |
			    	      BUTTONIDCMP |
				      LISTVIEWIDCMP |
				      IDCMP_CLOSEWINDOW,
			    TAG_DONE);

    SetMenuStrip(win, menus);
    
    RenderRegisterTab(win->RPort, &reg, TRUE);
}

/*********************************************************************************************/

static void KillWin(void)
{
    pagetable[reg.active].handler(PAGECMD_REMGADGETS, 0);
    
    if (win) CloseWindow(win);
}

/*********************************************************************************************/

static void ActivatePage(WORD which)
{
    if (which == activetab) return;
    
    pagetable[activetab].handler(PAGECMD_REMGADGETS, 0);
    
    SetDrMd(win->RPort, JAM1),
    SetAPen(win->RPort, dri->dri_Pens[BACKGROUNDPEN]);
    RectFill(win->RPort, pages_left, pages_top, pages_left + pages_width - 1, pages_top + pages_height - 1);
    
    activetab = which;

    pagetable[activetab].handler(PAGECMD_ADDGADGETS, 0);
    
}

/*********************************************************************************************/

static void HandleAll(void)
{
    struct IntuiMessage *msg;
    struct MenuItem     *item;
    UWORD               men;
    BOOL                quitme = FALSE;
    
    while (!quitme)
    {
	WaitPort(win->UserPort);
	
	while((msg = GT_GetIMsg(win->UserPort)))
	{
	    if (HandleRegisterTabInput(&reg, msg))
	    {
	    	ActivatePage(reg.active);
	    }
	    else if (pagetable[activetab].handler(PAGECMD_HANDLEINPUT, (IPTR)msg))
	    {
	    }
	    else switch (msg->Class)
	    {
		case IDCMP_CLOSEWINDOW:
		    quitme = TRUE;
		    break;
		
		case IDCMP_VANILLAKEY:
		    switch(msg->Code)
		    {
			case 27: /* ESC */
			    quitme = TRUE;
			    break;
			    
		    } /* switch(msg->Code) */
		    break;

		case IDCMP_MENUPICK:
		    men = msg->Code;            
		    while(men != MENUNULL)
		    {
			if ((item = ItemAddress(menus, men)))
			{
			    switch((ULONG)GTMENUITEM_USERDATA(item))
			    {
    	    	    	    	default:
				    break;
				
			    } /* switch(GTMENUITEM_USERDATA(item)) */
			    
			    men = item->NextSelect;
			} else {
			    men = MENUNULL;
			}
			
		    } /* while(men != MENUNULL) */
		    break;
		    		
	    } /* else switch (msg->Class) */
	    
	    GT_ReplyIMsg(msg);
	    
	} /* while((msg = GT_GetIMsg(win->UserPort))) */
	
    } /* while (!quitme) */
}

/*********************************************************************************************/

int main(void)
{
    InitLocale("Sys/localeprefs.catalog", 1);
    InitMenus();
    OpenLibs();
    GetArguments();
    GetVisual();
    MakeMenus();
    MakePages();
    LayoutGUI();
    MakeWin();
    HandleAll();
    Cleanup(NULL);
    
    return 0;
}

/*********************************************************************************************/


