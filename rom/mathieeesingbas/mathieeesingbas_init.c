/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Init of mathieeesingbas.library
    Lang: english
*/

#define AROS_ALMOST_COMPATIBLE
//#include <utility/utility.h>
#include "mathieeesingbas_intern.h"
#include "libdefs.h"

#ifdef SysBase
#   undef SysBase
#endif
#ifdef ExecBase
#   undef ExecBase
#endif

/* Customize libheader.c */
#define LC_SYSBASE_FIELD(lib)	(((struct LIBBASETYPEPTR       )(lib))->misb_SysBase)
#define LC_SEGLIST_FIELD(lib)   (((struct LIBBASETYPEPTR       )(lib))->misb_SegList)
#define LC_RESIDENTNAME		Mathieeesingbas_resident
#define LC_RESIDENTFLAGS	RTF_AUTOINIT|RTF_COLDSTART
#define LC_RESIDENTPRI		101
#define LC_LIBBASESIZE		sizeof(struct LIBBASETYPE)
#define LC_LIBHEADERTYPEPTR	struct LIBBASETYPEPTR
#define LC_LIB_FIELD(lib)       (((struct LIBBASETYPEPTR)(lib))->LibNode)

#define LC_NO_OPENLIB
#define LC_NO_CLOSELIB
#define LC_NO_EXPUNGELIB
#define LC_STATIC_INITLIB

#include <libcore/libheader.c>

struct ExecBase   * SysBase; /* global variable */

ULONG SAVEDS L_InitLib (LC_LIBHEADERTYPEPTR lh)
{
    SysBase = lh->misb_SysBase;

    return TRUE;
} /* L_InitLib */



