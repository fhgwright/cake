/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: X11 hidd. Connects to the X server and receives events.
    Lang: English.
*/

#define AROS_ALMOST_COMPATIBLE 1

#include <hidd/unixio.h>
#include <hidd/hidd.h>

#include <proto/exec.h>
#include <proto/oop.h>
#include <proto/utility.h>

#include <oop/ifmeta.h>

#include <dos/dos.h>

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/resident.h>

#include <utility/utility.h>



#include <aros/asmcall.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>


#include "x11.h"

#define DEBUG 0
#include <aros/debug.h>


struct x11_data
{
    ULONG dummy;
    
};





static struct abdescr attrbases[] =
{
    { NULL, NULL }
};

#define XSD(cl) ((struct x11_staticdata *)cl->UserData)
#define OOPBase		((struct Library *)XSD(cl)->oopbase)
#define UtilityBase	((struct Library *)XSD(cl)->utilitybase)

static Object *x11_new(Class *cl, Object *o, struct pRoot_New *msg)
{
    o = (Object *)DoSuperMethod(cl, o, (Msg) msg);
    if (o)
    {
    }
    return o;
}

#define IS_X11_ATTR(attr, idx) (( (idx) = (attr) - HiddX11AB) < num_Hidd_X11_Attrs)

static VOID x11_dispose(Class *cl, Object *o, Msg msg)
{
    DoSuperMethod(cl, o, (Msg)msg);
    return;
}

static VOID x11_get(Class *cl, Object *o, struct pRoot_Get *msg)
{
    return;
}


#undef XSD
#define XSD(cl) xsd
#undef SysBase



#define NUM_ROOT_METHODS 3
#define NUM_X11_METHODS 0

Class *init_x11class (struct x11_staticdata *xsd)
{
    Class *cl = NULL;

    struct MethodDescr root_descr[NUM_ROOT_METHODS + 1] = 
    {
    	{METHODDEF(x11_new),		moRoot_New},
    	{METHODDEF(x11_dispose),	moRoot_Dispose},
    	{METHODDEF(x11_get),		moRoot_Get},
	{NULL, 0UL}
    };
    
    struct MethodDescr x11hidd_descr[NUM_X11_METHODS + 1] = 
    {
	{NULL, 0UL}
    };
    
    struct InterfaceDescr ifdescr[] =
    {
    	{root_descr, 	IID_Root, 	NUM_ROOT_METHODS},
    	{x11hidd_descr, IID_Hidd_X11, 	NUM_X11_METHODS},
	{NULL, NULL, 0}
    };
    
    AttrBase MetaAttrBase = ObtainAttrBase(IID_Meta);
	
    struct TagItem tags[] =
    {
	{ aMeta_SuperID,		(IPTR)CLID_Hidd },
	{ aMeta_InterfaceDescr,		(IPTR)ifdescr},
	{ aMeta_InstSize,		(IPTR)sizeof (struct x11_data) },
	{TAG_DONE, 0UL}
    };

    EnterFunc(bug("X11HiddClass init\n"));
    
    if (MetaAttrBase)
    {
    	cl = NewObject(NULL, CLID_HiddMeta, tags);
    	if(cl)
    	{
	    cl->UserData = (APTR)xsd;
	    xsd->x11class = cl;
	    
	    if (obtainattrbases(attrbases, OOPBase))
	    {
		D(bug("X11HiddClass ok\n"));
		
	    	AddClass(cl);
	    }
	    else
	    {
	    	free_x11class(xsd);
		cl = NULL;
	    }
	}
	/* Don't need this anymore */
	ReleaseAttrBase(IID_Meta);
    }
    return cl;
}




/*************** free_x11class()  **********************************/
VOID free_x11class(struct x11_staticdata *xsd)
{
    EnterFunc(bug("free_x11class(xsd=%p)\n", xsd));

    if(xsd)
    {

        RemoveClass(xsd->x11class);
	
        if(xsd->x11class) DisposeObject((Object *) xsd->x11class);
        xsd->x11class = NULL;
	
	releaseattrbases(attrbases, OOPBase);

    }

    ReturnVoid("free_x11class");
}


static int unixio_callback(int displayfd, struct x11_staticdata *xsd)
{
    int pending;
    
LX11    
    pending = XPending(xsd->display);
UX11


    return pending;
}

VOID x11task_entry(struct x11task_params *xtp)
{
    HIDD *unixio;
    struct x11_staticdata *xsd = xtp->xsd;
    
    unixio = (HIDD)New_UnixIO(OOPBase);
    if (unixio)
    {
	Signal(xtp->parent, xtp->ok_signal);
    }
    else
    {
    	Signal(xtp->parent, xtp->fail_signal);
    }
    
D(bug("Entering input loop, sema owner=%p, self=%p, nestcnt=%d, qcnt=%d\n"
	, xsd->x11sema.ss_Owner, FindTask(NULL), xsd->x11sema.ss_NestCount
	, xsd->x11sema.ss_QueueCount));
	


    for (;;)
    {
        int ret;
	XEvent event;
	
	
    	ret = (int)Hidd_UnixIO_Wait( unixio
			, ConnectionNumber( xsd->display )
			, vHidd_UnixIO_Read
			, unixio_callback
			, (APTR)xsd );
			
			
D(bug("Got input from unixio\n"));
			
	if (ret != 0)
	{
	    continue;
	}

 	for (;;)	    
	{
	    BOOL window_found = FALSE;
	    struct xwinnode *node;
	    int pending;


LX11	
	    pending = XPending (xsd->display);
UX11	    
	    if (pending == 0)
	    	break;

	
LX11
D(bug("Doing XNextEvent\n"));
	    XNextEvent (xsd->display, &event);
D(bug("Done XNextEvent()\n"));	    
UX11

	    D(bug("Got Event for X=%d\n", event.xany.window));

	    if (event.type == MappingNotify)
	    {
LX11
		XRefreshKeyboardMapping ((XMappingEvent*)&event);
UX11
		continue;
	    }
	    
	    ObtainSemaphoreShared( &xsd->winlistsema );
	    
	    ForeachNode( &xsd->xwindowlist, node)
	    {
	        if (node->xwindow == event.xany.window)
		{
		    window_found = TRUE;
		    break;
		}
	    }
	    
	    ReleaseSemaphore( &xsd->winlistsema );
	    
	    
	    
	    if (window_found)
	    {
	        D(bug("Got event for window %x\n", event.xany.window));
	    	switch (event.type)
	    	{
	    	case GraphicsExpose:
	    	case Expose:
		    break;

	        case ConfigureNotify:
		    break;

	    	case ButtonPress:
	        case ButtonRelease:
	    	case MotionNotify:
		    D(bug("Motionnotify event\n"));

		
	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->mousehidd)
			Hidd_X11Mouse_HandleEvent(xsd->mousehidd, &event);
		    ReleaseSemaphore( &xsd->sema );
		    break;

	    	case KeyPress: 
	    	case KeyRelease:
		    D(bug("Keypress/keyreleas\n"));
	    	    ObtainSemaphoreShared( &xsd->sema );
		    if (xsd->kbdhidd)
		    {
		        D(bug("kbdhidd present, calling it\n"));
			Hidd_X11Kbd_HandleEvent(xsd->kbdhidd, &event);
		    }
	    	
		    ReleaseSemaphore( &xsd->sema );
		    break;


	   	case EnterNotify:
		    break;

	    	case LeaveNotify:
		    break;

	        } /* switch (X11 event type) */
		
	    } /* if (is event for HIDD window) */


    	} /* while (events from X)  */
    	
    } /* Forever */
    
}

struct Task *create_x11task( struct x11task_params *params, struct ExecBase *ExecBase)
{
    struct Task *task;
    APTR stack;
    
    task = AllocMem(sizeof (struct Task), MEMF_PUBLIC|MEMF_CLEAR);
    if (task)
    {
    	NEWLIST(&task->tc_MemEntry);
    	task->tc_Node.ln_Type=NT_TASK;
    	task->tc_Node.ln_Name= XTASK_NAME;
    	task->tc_Node.ln_Pri = XTASK_PRIORITY;

    	stack=AllocMem(XTASK_STACKSIZE, MEMF_PUBLIC);
    	if(stack != NULL)
    	{
	    task->tc_SPLower=stack;
	    task->tc_SPUpper=(BYTE *)stack + XTASK_STACKSIZE;

#if AROS_STACK_GROWS_DOWNWARDS
	    task->tc_SPReg = (BYTE *)task->tc_SPUpper-SP_OFFSET-sizeof(APTR);
	    ((APTR *)task->tc_SPUpper)[-1] = params;
#else
	    task->tc_SPReg=(BYTE *)task->tc_SPLower-SP_OFFSET + sizeof(APTR);
	    *(APTR *)task->tc_SPLower = params;
#endif

	    
	    if(AddTask(task, x11task_entry, NULL) != NULL)
	    {
	    	/* Everything went OK. Wait for task to initialize */
		ULONG sigset;
		
		sigset = Wait( params->ok_signal | params->fail_signal );
		if (sigset & params->ok_signal)
		{
		    return task;
		}
	    }	
	    FreeMem(stack, XTASK_STACKSIZE);
    	}
        FreeMem(task,sizeof(struct Task));
    }
    
    return NULL;

}

