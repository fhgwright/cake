/*
    Copyright (C) 1995-1997 AROS - The Amiga Replacement OS
    $Id$

    Desc: exec.library resident and initialization.
    Lang: english
*/
#define AROS_ALMOST_COMPATIBLE

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <dos/dosextens.h>
#include <stdlib.h>

#include <aros/arossupportbase.h>
#include <aros/machine.h>
#include <aros/asmcall.h>

#include <aros/debug.h>

#include <proto/arossupport.h>
#include <proto/exec.h>

#include "exec_private.h"
#include "libdefs.h"

static const UBYTE name[];
static const UBYTE version[];
extern const char END;
static struct ExecBase *AROS_SLIB_ENTRY(init,BASENAME)();

const struct Resident Exec_resident =
{
    RTC_MATCHWORD,
    (struct Resident *)&Exec_resident,
    (APTR)&END,
    RTF_SINGLETASK,
    LIBVERSION,
    NT_LIBRARY,
    105,
    (STRPTR)name,
    (STRPTR)&version[6],
    &AROS_SLIB_ENTRY(init,BASENAME)
};

static const UBYTE name[] = LIBNAME;
static const UBYTE version[] = VERSION;

extern void debugmem(void);

/* FIXME: This is public to allow PrepareExecBase() to work */
struct AROSSupportBase AROSSupportBase;
struct ExecBase *SysBase;

void _aros_not_implemented(void)
{
    kprintf("This function is not implemented.\n");
}

void aros_print_not_implemented(char *name)
{
    kprintf("The function %s is not implemented.\n", name);
}

/* IntServer:
    This interrupt handler will send an interrupt to a series of queued
    interrupt servers. Servers should return D0 != 0 (Z clear) if they
    believe the interrupt was for them, and no further interrupts will
    be called. This will only check the value in D0 for non-m68k systems,
    however it SHOULD check the Z-flag on 68k systems.

    Hmm, in that case I would have to separate it from this file in order
    to replace it...
*/
AROS_UFH5(static void, IntServer,
    AROS_UFHA(ULONG, intMask, D0),
    AROS_UFHA(struct Custom *, custom, A0),
    AROS_UFHA(struct List *, intList, A1),
    AROS_UFHA(APTR, intCode, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    struct Interrupt * irq;

    ForeachNode(intList, irq)
    {
	if( AROS_UFC4(int, irq->is_Code,
		AROS_UFCA(struct Custom *, custom, A0),
		AROS_UFCA(APTR, irq->is_Data, A1),
		AROS_UFCA(APTR, irq->is_Code, A5),
		AROS_UFCA(struct ExecBase *, SysBase, A6)
	))
	    break;
    }
}

AROS_UFH4(int, Dispatcher,
    AROS_UFHA(struct Custom *, custom, A0),
    AROS_UFHA(APTR, is_Data, A1),
    AROS_UFHA(APTR, is_Code, A5),
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    /* Check if a task switch is necessary */
    if( SysBase->TaskReady.lh_Head->ln_Succ != NULL
	&& SysBase->ThisTask->tc_Node.ln_Pri <=
	   ((struct Task *)SysBase->TaskReady.lh_Head)->tc_Node.ln_Pri)
    {
	/* Check if task switch is possible */
	if( SysBase->TDNestCnt < 0 )
	{
	    if( SysBase->ThisTask->tc_State == TS_RUN )
	    {
		SysBase->ThisTask->tc_State = TS_READY;
		Reschedule(SysBase->ThisTask);
		SysBase->AttnResched |= 0x8000;
	    }
	    else if( SysBase->ThisTask->tc_State == TS_REMOVED )
		SysBase->AttnResched |= 0x8000;
	}
	else
	    SysBase->AttnResched |= 0x80;
    }
    /* This make the int handler continue with the rest of the ints. */
    return 0;
} /* Dispatcher */

void idleTask(struct ExecBase *SysBase)
{
    struct Task *inputDevice = NULL;
    struct Task *idle = FindTask (NULL);

    while(1)
    {
	/* If the input device exists, we should signal it */
	if( inputDevice )
	{
	    Signal(inputDevice, SIGBREAKF_CTRL_F);
	}
	else
	{
	    /* Does somebody want the input device prodded, if so
	       then we had better be told
	    */
	    if( SetSignal(0,0) & SIGBREAKF_CTRL_F )
		inputDevice = FindTask("input.device");
	}

	/* Test if there are any other tasks in the ready queue */
	if( !IsListEmpty(&SysBase->TaskReady) )
	{
	    /* TODO Doesn't work, yet Reschedule(FindTask(NULL)); */
	    Forbid ();
	    idle->tc_State = TS_READY;
	    AddTail (&SysBase->TaskReady, &idle->tc_Node);
	    Permit ();
	    Switch();
	}
    }
}

AROS_UFH1(void, idleCount,
    AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    /* This keeps track of how many times the idle task becomes active.
	Apart from also testing the tc_Launch vector, it also keeps a
	count of how many times we've gone idle since startup.
    */
    SysBase->IdleCount++;
}

AROS_LH2(struct LIBBASETYPE *, init,
    AROS_LHA(ULONG, dummy, D0),
    AROS_LHA(BPTR, segList, A0),
    struct ExecBase *, SysBase, 0, BASENAME)
{
    AROS_LIBFUNC_INIT

    /* We have been entered by a call to InitCode(RTF_SINGLETASK,0); */

    /* Create boot task.  Sigh, we actually create a Process sized Task,
	since DOS needs to call things which think it has a Process and
	we don't want to overwrite memory with something strange do we?

	We do this until at least we can boot dos more cleanly.
    */
    {
	struct Task    *t;
	struct MemList *ml;

	ml = (struct MemList *)AllocMem(sizeof(struct MemList), MEMF_PUBLIC|MEMF_CLEAR);
	t  = (struct Task *)   AllocMem(sizeof(struct Process), MEMF_PUBLIC|MEMF_CLEAR);

	if( !ml || !t )
	{
	    kprintf("ERROR: Cannot create Boot Task!\n");
	    exit(20);
	}
	ml->ml_NumEntries = 1;
	ml->ml_ME[0].me_Addr = t;
	ml->ml_ME[0].me_Length = sizeof(struct Process);

	NEWLIST(&t->tc_MemEntry);
	NEWLIST(&((struct Process *)t)->pr_MsgPort.mp_MsgList);
	AddHead(&t->tc_MemEntry,&ml->ml_Node);

	t->tc_Node.ln_Name = "Boot Task";
	t->tc_Node.ln_Pri = 0;
	t->tc_State = TS_RUN;
	t->tc_SigAlloc = 0xFFFF;
	t->tc_SPLower = 0;	    /* This is the system's stack */
	t->tc_SPUpper = (APTR)~0UL;

	SysBase->ThisTask = t;
    }

    {
	/* Add idle task */
	struct Task *t;
	struct MemList *ml;
	UBYTE *s;

	/* Allocate MemEntry for this task and stack */
	ml = (struct MemList *)AllocMem(sizeof(struct MemList)+sizeof(struct MemEntry),
					MEMF_PUBLIC|MEMF_CLEAR);
	t = (struct Task *)    AllocMem(sizeof(struct Task), MEMF_CLEAR|MEMF_PUBLIC);
	s = (UBYTE *)          AllocMem(AROS_STACKSIZE,      MEMF_CLEAR|MEMF_PUBLIC);

	if( !ml || !t || !s )
	{
	    kprintf("ERROR: Cannot create Idle Task!\n");
	    exit(20);
	}

	ml->ml_NumEntries = 2;
	ml->ml_ME[0].me_Addr = t;
	ml->ml_ME[0].me_Length = sizeof(struct Task);
	ml->ml_ME[1].me_Addr = s;
	ml->ml_ME[1].me_Length = AROS_STACKSIZE;

	NEWLIST(&t->tc_MemEntry);
	AddHead(&t->tc_MemEntry, &ml->ml_Node);
	t->tc_SPLower = s;
	t->tc_SPUpper = s + AROS_STACKSIZE;

	/* Pass SysBase in on the stack */
	t->tc_SPReg = &(((struct ExecBase *)(s + AROS_STACKSIZE))[-1]);
	*((struct ExecBase **)t->tc_SPReg) = SysBase;

	t->tc_Node.ln_Name = "Idle Task";
	t->tc_Node.ln_Pri = -128;
	t->tc_Launch = &idleCount;
	t->tc_Flags = TF_LAUNCH;
	AddTask(t, &idleTask, NULL);
    }

    {
	/* Install the interrupt servers */
	int i;
	for(i=0; i < 16; i++)
	    if( (1<<i) & (INTF_PORTS|INTF_COPER|INTF_VERTB|INTF_EXTER|INTF_SETCLR))
	    {
		struct Interrupt *is;
		struct SoftIntList *sil;
		is = AllocMem(sizeof(struct Interrupt) + sizeof(struct SoftIntList),
				MEMF_CLEAR|MEMF_PUBLIC);
		if( is == NULL )
		{
		    kprintf("ERROR: Cannot install Interrupt Servers!\n");
		    exit(20);
		}
		sil = (struct SoftIntList *)((struct Interrupt *)is + 1);

		is->is_Code = &IntServer;
		is->is_Data = sil;
		NEWLIST((struct List *)sil);
		SetIntVector(i,is);
	    }
    }

    {
	/* Install the task dispatcher */
	struct Interrupt *is;
	is = (struct Interrupt *)AllocMem(sizeof(struct Interrupt), MEMF_CLEAR|MEMF_PUBLIC);
	if(!is)
	{
	    kprintf("ERROR: Cannot install Task Dispatcher!\n");
	    exit(20);
	}
	is->is_Code = (void (*)())&Dispatcher;
	AddIntServer(INTB_VERTB,is);
    }

    /* We now start up the interrupts */
    Enable();

#ifdef DEBUG
    debugmem();
#endif

    /* This will cause everything else to run. This call will not return.
	This is because it eventually falls into strap, which will call
	the bootcode, which itself is not supposed to return. It is up
	to the DOS (whatever it is) to Permit(); RemTask(NULL);
    */
    InitCode(RTF_COLDSTART, 0);

    /* There had better be some kind of task waiting to run. */
    return NULL;
    AROS_LIBFUNC_EXIT
}

const char END = 1;
