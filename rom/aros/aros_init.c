/*
    Copyright (C) 1995-98 AROS - The Amiga Replacement OS
    $Id$

    Desc: aros.library Resident and initialization.
    Lang: english
*/

#include <exec/types.h>
#include <exec/resident.h>
#include <proto/exec.h>

#include "aros_intern.h"

#define DEBUG 0
#include <aros/debug.h>
#undef kprintf

#define INIT AROS_SLIB_ENTRY(init,Aros)

extern const UBYTE name[];
extern const UBYTE version[];
extern const APTR inittabl[4];
extern void *const FUNCTABLE[];
struct LIBBASETYPE *INIT();
extern const char END;

int Aros_entry(void)
{
    /* If the library was executed by accident return error code. */
    return -1;
}

const struct Resident Aros_resident=
{
    RTC_MATCHWORD,
    (struct Resident *)&Aros_resident,
    (APTR)&END,
    RTF_AUTOINIT|RTF_COLDSTART,
    LIBVERSION,
    NT_LIBRARY,
    102,		/* Immediately after utility.library */
    (STRPTR)name,
    (STRPTR)&version[6],
    (ULONG *)inittabl
};

const UBYTE name[]=LIBNAME;

const UBYTE version[]=VERSION;

const APTR inittabl[4]=
{
    (APTR)sizeof(struct ArosBase),
    (APTR)FUNCTABLE,
    NULL,
    &INIT
};

AROS_LH2(struct LIBBASETYPE *, init,
    AROS_LHA(struct LIBBASETYPE *, LIBBASE, D0),
    AROS_LHA(BPTR,               segList,   A0),
    struct ExecBase *, sysBase, 0, BASENAME)
{
    AROS_LIBFUNC_INIT

    /* Store arguments */
    LIBBASE->aros_sysBase=sysBase;
    LIBBASE->aros_segList=segList;

    /* Set up ArosBase */
    LIBBASE->aros_LibNode.lib_Node.ln_Pri = 0;
    LIBBASE->aros_LibNode.lib_Node.ln_Type = NT_LIBRARY;
    (UBYTE const *)ArosBase->aros_LibNode.lib_Node.ln_Name = name;
    LIBBASE->aros_LibNode.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    LIBBASE->aros_LibNode.lib_Version = LIBVERSION;
    LIBBASE->aros_LibNode.lib_Revision = LIBREVISION;
    (UBYTE const *)LIBBASE->aros_LibNode.lib_IdString = &version[6];

    D(bug("aros.library starting...\n"));

    /* You would return NULL if the init failed */
    return LIBBASE;
    AROS_LIBFUNC_EXIT
}

AROS_LH1(struct LIBBASETYPE *, open,
 AROS_LHA(ULONG, version, D0),
	   struct LIBBASETYPE *, LIBBASE, 1, BASENAME)
{
    AROS_LIBFUNC_INIT

    if(!(LIBBASE->aros_utilityBase))
    {
	if(!(LIBBASE->aros_utilityBase = OpenLibrary("utility.library", 37)))
	    return NULL;
    }

    /* I have one more opener. */
    LIBBASE->aros_LibNode.lib_OpenCnt++;
    LIBBASE->aros_LibNode.lib_Flags&=~LIBF_DELEXP;

    /* You would return NULL if the open failed. */
    return LIBBASE;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, close,
	   struct LIBBASETYPE *, LIBBASE, 2, BASENAME)
{
    AROS_LIBFUNC_INIT

    /* I have one fewer opener. */
    if(!--LIBBASE->aros_LibNode.lib_OpenCnt)
    {
	    return expunge();
    }
    return 0;
    AROS_LIBFUNC_EXIT
}

/* aros.library must not ever be expunged! */

AROS_LH0(BPTR, expunge,
	   struct LIBBASETYPE *, LIBBASE, 3, BASENAME)
{
    AROS_LIBFUNC_INIT

    /* Test for openers. */
    if(LIBBASE->aros_LibNode.lib_OpenCnt)
    {
	/* Set the delayed expunge flag and return. */
	LIBBASE->aros_LibNode.lib_Flags|=LIBF_DELEXP;
    }

    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null,
	    struct LIBBASETYPE *, LIBBASE, 4, BASENAME)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}
