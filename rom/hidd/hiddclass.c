/*
    Copyright (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: Main class for HIDD.
    Lang: english
*/

#define AROS_ALMOST_COMPATIBLE
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/alerts.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>
#include <oop/oop.h>
#include <hidd/hidd.h>

#include <proto/exec.h>
#include <proto/oop.h>
#include <proto/utility.h>

#ifdef _AROS
#include <aros/asmcall.h>
#endif /* _AROS */

#define SDEBUG 0
#define DEBUG 0
#include <aros/debug.h>

static const UBYTE name[];
static const UBYTE version[];
static ULONG AROS_SLIB_ENTRY(init,HIDD)();
extern const char HIDD_End;

int entry(void)
{
    return -1;
}

const struct Resident HIDD_resident =
{
    RTC_MATCHWORD,
    (struct Resident *)&HIDD_resident,
    (APTR)&HIDD_End,
    RTF_COLDSTART,
    41,
    NT_UNKNOWN,
    90, /* Has to be after OOP */
    (UBYTE *)name,
    (UBYTE *)version,
    (APTR)&AROS_SLIB_ENTRY(init,HIDD)
};

static const UBYTE name[] = "hiddclass";
static const UBYTE version[] = "hiddclass 41.1 (23.10.1997)\r\n";

static const char unknown[] = "--unknown device--";


static AttrBase HiddAttrBase;

#define IS_HIDD_ATTR(attr, idx) ((idx = attr - HiddAttrBase) < num_Hidd_Attrs)

/************************************************************************/

struct HIDDData
{
    UWORD	hd_Type;
    UWORD	hd_SubType;
    ULONG	hd_Producer;
    STRPTR	hd_Name;
    STRPTR	hd_HWName;
    BOOL	hd_Active;
    UWORD	hd_Locking;
    ULONG	hd_Status;
    ULONG	hd_ErrorCode;
};

/* Static Data for the hiddclass. */
struct HCD
{
    struct Library		*UtilityBase;
    struct Library		*OOPBase;
    struct MinList		 hiddList;
    struct SignalSemaphore	 listLock;
};

#define OOPBase	(((struct HCD *)cl->UserData)->OOPBase)
#define UtilityBase	(((struct HCD *)cl->UserData)->UtilityBase)

/* Implementation of root HIDD class methods. */
static VOID hidd_set(Class *cl, Object *o, struct pRoot_Set *msg);


/******************
**  HIDD::New()  **
******************/
static Object *hidd_new(Class *cl, Object *o, struct pRoot_New *msg)
{
    EnterFunc(bug("HIDD::New(cl=%s)\n", cl->ClassNode.ln_Name));
    D(bug("DoSuperMethod:%p\n", cl->DoSuperMethod));
    o = (Object *)DoSuperMethod(cl, o, (Msg)msg);
    if(o)
    {
    	struct HIDDData *hd;
        struct TagItem *list = msg->attrList;
	struct pRoot_Set set_msg;
	
	hd = INST_DATA(cl, o);

	/*  Initialise the HIDD class. These fields are publicly described
	    as not being settable at Init time, however it is the only way to
	    get proper abstraction if you ask me. Plus it does reuse code
	    in a nice way.

	    To pass these into the init code I would recommend that your
	    pass in a TagList of your tags, which is linked to the user's
	    tags by a TAG_MORE. This way you will prevent them from setting
	    these values.
	*/

	hd->hd_Type 	= GetTagData(aHidd_Type, 	0, list);
	hd->hd_SubType 	= GetTagData(aHidd_SubType, 	0, list);
	hd->hd_Producer = GetTagData(aHidd_Producer, 	0, list);
	
	hd->hd_Name 	= (STRPTR)GetTagData(aHidd_Name, 	(IPTR)unknown,	list);
	hd->hd_HWName 	= (STRPTR)GetTagData(aHidd_HardwareName,(IPTR)unknown,	list);
	
	hd->hd_Status 	= GetTagData(aHidd_Status, 	vHidd_StatusUnknown, 	list);
	hd->hd_Locking 	= GetTagData(aHidd_Locking, 	vHidd_LockShared, 	list);
	hd->hd_ErrorCode= GetTagData(aHidd_ErrorCode, 	0, list);

	hd->hd_Active 	= TRUE; /* Set default, GetTagData() comes later */
	
	/* Use OM_SET to set the rest */


	set_msg.attrList = msg->attrList;
	hidd_set(cl, o, &set_msg);
	
	
    }
    
    ReturnPtr("HIDD::New", Object *, o);
}


/******************
**  HIDD::Set()  **
******************/

static VOID hidd_set(Class *cl, Object *o, struct pRoot_Set *msg)
{

    struct TagItem *tstate = msg->attrList;
    struct TagItem *tag;
    struct HIDDData *hd = INST_DATA(cl, o);

    while((tag = NextTagItem(&tstate)))
    {
    	ULONG idx;
	
    	if (IS_HIDD_ATTR(tag->ti_Tag, idx))
	{
	    switch(idx)
	    {
		case aoHidd_Active:
	    	    hd->hd_Active = tag->ti_Data;
	    	    break;
		    
	    }
	}
    }
    return;
}

/******************
**  HIDD::Get()  **
******************/
static VOID hidd_get(Class *cl, Object *o, struct pRoot_Get *msg)
{
    struct HIDDData *hd = INST_DATA(cl, o);
    ULONG idx;
    

    if (IS_HIDD_ATTR(msg->attrID, idx))
    {
    	switch (idx)
	{
	case aoHidd_Type:
	    *msg->storage = hd->hd_Type;
	    break;

	case aoHidd_SubType:
	    *msg->storage = hd->hd_SubType;
	    break;

	case aoHidd_Producer:
	    *msg->storage = hd->hd_Producer;
	    break;

	case aoHidd_Name:
	    *msg->storage = (IPTR)hd->hd_Name;
	    break;

	case aoHidd_HardwareName:
	    *msg->storage = (IPTR)hd->hd_HWName;
	    break;

	case aoHidd_Active:
	    *msg->storage = hd->hd_Active;
	    break;

	case aoHidd_Status:
	    *msg->storage = hd->hd_Status;
	    break;

	case aoHidd_ErrorCode:
	    *msg->storage = hd->hd_ErrorCode;
	    break;

	case aoHidd_Locking:
	    *msg->storage = hd->hd_Locking;
	    break;
	
	}
    }
    
    return;

}


/***********************************
**  Unimplemented methods 
*/




/*    switch(msg->MethodID)
    {
    case OM_NEW:
	retval = DoSuperMethodA(cl, o, msg);
	if(!retval)
	    break;

	hd = INST_DATA(cl, retval);

	if( hd != NULL)
	{
	    struct TagItem *list = ((struct opSet *)msg)->ops_AttrList;
	    hd->hd_Type = GetTagData(aHidd_Type, 0, list);
	    hd->hd_SubType = GetTagData(aHidd_SubType, 0, list);
	    hd->hd_Producer = GetTagData(aHidd_Producer, 0, list);
	    hd->hd_Name = (STRPTR)GetTagData(aHidd_Name, (IPTR)unknown, list);
	    hd->hd_HWName = (STRPTR)GetTagData(aHidd_HardwareName, (IPTR)unknown, list);
	    hd->hd_Active = TRUE; 
	    hd->hd_Status = GetTagData(aHidd_Status, HIDDV_StatusUnknown, list);
	    hd->hd_ErrorCode = GetTagData(aHidd_ErrorCode, 0, list);
	    hd->hd_Locking = GetTagData(aHidd_Locking, HIDDV_LockShared, list);
	}

    case OM_SET:
    {
	struct TagItem *tstate = ((struct opSet *)msg)->ops_AttrList;
	struct TagItem *tag;

	while((tag = NextTagItem(&tstate)))
	{
	    switch(tag->ti_Tag)
	    {
	    case aHidd_Active:
		hd->hd_Active = tag->ti_Data;
		break;
	    }
	}
	break;
    }


    case OM_GET:
    {
	switch(((struct opGet *)msg)->opg_AttrID)
	{
	case aHidd_Type:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_Type;
	    break;

	case aHidd_SubType:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_SubType;
	    break;

	case aHidd_Producer:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_Producer;
	    break;

	case aHidd_Name:
	    *((struct opGet *)msg)->opg_Storage = (IPTR)hd->hd_Name;
	    break;

	case aHidd_HardwareName:
	    *((struct opGet *)msg)->opg_Storage = (IPTR)hd->hd_HWName;
	    break;

	case aHidd_Active:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_Active;
	    break;

	case aHidd_Status:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_Status;
	    break;

	case aHidd_ErrorCode:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_ErrorCode;
	    break;

	case aHidd_Locking:
	    *((struct opGet *)msg)->opg_Storage = hd->hd_Locking;
	    break;
	}
    }

*/


    /* These are the "hiddclass" methods. */

    /*	These two are invalid, since we don't have anything to get
	from a class, so the superclass should handle these.

	This is especially the case since the only place that we can
	get the information for these methods is from an object, but
	we don't have any objects if this method is called.
    */
/*    case HIDDM_Meta_Get:
    case HIDDM_Meta_MGet:
	retval = 0;
	break;
*/
    /*	Yet to determine the semantics of these so we just let
	them return 0 for now.
    */
/*    case HIDDM_BeginIO:
    case HIDDM_AbortIO:
	retval = 0;
	break;

    case HIDDM_LoadConfigPlugin:
    case HIDDM_Lock:
    case HIDDM_Unlock:
	retval = NULL;
	break;

    case HIDDM_AddHIDD:
    {

	Class *hc = ((hmAdd *)msg)->hma_Class;

	if( (hc->cl_Flags & CLF_INLIST) == 0 )
	{

	    ObtainSemaphore(&((struct HCD *)cl->cl_UserData)->listLock);
	    AddTail(
		(struct List *)&((struct HCD *)cl->cl_UserData)->hiddList,
		(struct Node *)hc
	    );
	    ReleaseSemaphore(&((struct HCD *)cl->cl_UserData)->listLock);

	    hc->cl_Flags |= CLF_INLIST;
	    retval = TRUE;
	}
	break;
    }

    case HIDDM_RemoveHIDD:
    {
	struct IClass *hc = ((hmAdd *)msg)->hma_Class;

	if( hc->cl_Flags & CLF_INLIST )
	{
	    ObtainSemaphore(&((struct HCD *)cl->cl_UserData)->listLock);
	    Remove((struct Node *)hc);
	    ReleaseSemaphore(&((struct HCD *)cl->cl_UserData)->listLock);
	    hc->cl_Flags &= ~CLF_INLIST;
	}
    }

    case OM_DISPOSE:

    default:
	retval = DoSuperMethodA(cl, o, msg);
    }

    return retval;
}
*/
/* This is the initialisation code for the HIDD class itself. */
#undef OOPBase
#undef UtilityBase

#define NUM_ROOT_METHODS 3
#define NUM_HIDD_METHODS 0

AROS_UFH3(static ULONG, AROS_SLIB_ENTRY(init, HIDD),
    AROS_UFHA(ULONG, dummy1, D0),
    AROS_UFHA(ULONG, dummy2, A0),
    AROS_UFHA(struct ExecBase *, SysBase, A6)
)
{
    struct Library *OOPBase;
    Class *cl;
    struct HCD *hcd;
    
    
    struct MethodDescr root_mdescr[NUM_ROOT_METHODS + 1] =
    {
    	{ (IPTR (*)())hidd_new,		moRoot_New		},
    	{ (IPTR (*)())hidd_set,		moRoot_Set		},
    	{ (IPTR (*)())hidd_get,		moRoot_Get		},
    	{ NULL, 0UL }
    };

    
    struct MethodDescr hidd_mdescr[NUM_HIDD_METHODS + 1] =
    {
    	{ NULL, 0UL }
    };
    
    struct InterfaceDescr ifdescr[] =
    {
    	{root_mdescr, IID_Root, NUM_ROOT_METHODS},
	{hidd_mdescr, IID_Hidd, NUM_HIDD_METHODS},
	{NULL, NULL, 0UL}
    
    };
    
    

    /*
	We map the memory into the shared memory space, because it is
	to be accessed by many processes, eg searching for a HIDD etc.

	Well, maybe once we've got MP this might help...:-)
    */
    hcd = AllocMem(sizeof(struct HCD), MEMF_CLEAR|MEMF_PUBLIC);
    if(hcd == NULL)
    {
	/* If you are not running from ROM, don't use Alert() */
	Alert(AT_DeadEnd | AG_NoMemory | AN_Unknown);
	return NULL;
    }

    NEWLIST(&hcd->hiddList);
    InitSemaphore(&hcd->listLock);

    OOPBase = hcd->OOPBase = OpenLibrary("oop.library", 0);
    if(hcd->OOPBase == NULL)
    {
	FreeMem(hcd, sizeof(struct HCD));
	Alert(AT_DeadEnd | AG_OpenLib | AN_Unknown | AO_Unknown);
	return NULL;
    }

    hcd->UtilityBase = OpenLibrary("utility.library",0);
    if(hcd->UtilityBase == NULL)
    {
	CloseLibrary(hcd->OOPBase);
	FreeMem(hcd, sizeof(struct HCD));
	Alert(AT_DeadEnd | AG_OpenLib | AN_Unknown | AO_UtilityLib);
	return NULL;
    }

    /* Create the class structure for the "hiddclass" */
    {
        AttrBase MetaAttrBase = GetAttrBase(IID_Meta);
        struct TagItem tags[] =
    	{
            {aMeta_SuperID,		(IPTR)CLID_Root},
	    {aMeta_InterfaceDescr,	(IPTR)ifdescr},
	    {aMeta_ID,			(IPTR)CLID_Hidd},
	    {aMeta_InstSize,		(IPTR)sizeof (struct HIDDData) },
	    {TAG_DONE, 0UL}
    	};
    
	cl = NewObject(NULL, CLID_HiddMeta, tags);
	if (cl == NULL)
	{
	    CloseLibrary(hcd->UtilityBase);
	    CloseLibrary(hcd->OOPBase);
	    FreeMem(hcd, sizeof(struct HCD));
	    Alert(AT_DeadEnd | AG_OpenLib | AN_Unknown | AO_Unknown);
	    return NULL;
        }
	
    }
    
    cl->UserData = hcd;
    HiddAttrBase = GetAttrBase(IID_Hidd);

    AddClass(cl);

    return TRUE;
}

const char HIDD_End = 0;
