/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Internal types and stuff for dos
    Lang: english
*/
#ifndef DOS_INTERN_H
#define DOS_INTERN_H

#include <dos/dosextens.h>
#include <dos/filesystem.h>

#ifdef SysBase
#undef SysBase
#endif
#define SysBase (DOSBase->dl_SysBase)
#ifdef UtilityBase
#undef UtilityBase
#endif
#define UtilityBase (DOSBase->dl_UtilityBase)
#ifdef TimerBase
#undef TimerBase
#endif
#define TimerBase (DOSBase->dl_TimerBase)

/* Needed for close() */
#define expunge() \
AROS_LC0(BPTR, expunge, struct DosLibrary *, DOSBase, 3, Dos)

struct DAList
{
    STRPTR *ArgBuf;
    UBYTE *StrBuf;
    STRPTR *MultVec;
};

struct EString
{
    LONG Number;
    STRPTR String;
};

extern struct EString EString[];

#ifndef EOF
#define EOF -1
#endif
#ifndef IOBUFSIZE
#define IOBUFSIZE 4096
#endif

struct vfp
{
    BPTR file;
    LONG count;
};

#define FPUTC(f,c) \
(((struct FileHandle *)BADDR(f))->fh_Flags&FHF_WRITE&& \
 ((struct FileHandle *)BADDR(f))->fh_Pos<((struct FileHandle *)BADDR(f))->fh_End? \
*((struct FileHandle *)BADDR(f))->fh_Pos++=c,0:FPutC(f,c))

LONG DoName(struct IOFileSys *iofs, STRPTR name, struct DosLibrary * DOSBase);
LONG DevName(STRPTR name, struct Device **devptr, struct DosLibrary * DOSBase);

struct marker
{
    UBYTE type; /* 0: Split 1: MP_NOT */
    STRPTR pat; /* Pointer into pattern */
    STRPTR str; /* Pointer into string */
};

struct markerarray
{
    struct markerarray *next;
    struct markerarray *prev;
    struct marker marker[128];
};

#define PUSH(t,p,s)                                                     \
{									\
    if(macnt==128)                                                      \
    {									\
	if(macur->next==NULL)                                           \
	{								\
	    macur->next=AllocMem(sizeof(struct markerarray),MEMF_ANY);  \
	    if(macur->next==NULL)                                       \
		ERROR(ERROR_NO_FREE_STORE);                             \
	    macur->next->prev=macur;					\
	}								\
	macur=macur->next;						\
	macnt=0;							\
    }									\
    macur->marker[macnt].type=(t);                                      \
    macur->marker[macnt].pat=(p);                                       \
    macur->marker[macnt].str=(s);                                       \
    macnt++;								\
}

#define POP(t,p,s)                      \
{					\
    macnt--;				\
    if(macnt<0)                         \
    {					\
	macnt=127;			\
	macur=macur->prev;		\
	if(macur==NULL)                 \
	    ERROR(0);                   \
    }					\
    (t)=macur->marker[macnt].type;      \
    (p)=macur->marker[macnt].pat;       \
    (s)=macur->marker[macnt].str;       \
}

#define MP_ESCAPE	0x81 /* Before characters in [0x81;0x8a] */
#define MP_MULT 	0x82 /* _#(_a) */
#define MP_MULT_END	0x83 /* #(a_)_ */
#define MP_NOT		0x84 /* _~(_a) */
#define MP_NOT_END	0x85 /* ~(a_)_ */
#define MP_OR		0x86 /* _(_a|b) */
#define MP_OR_NEXT	0x87 /* (a_|_b) */
#define MP_OR_END	0x88 /* (a|b_)_ */
#define MP_SINGLE	0x89 /* ? */
#define MP_ALL		0x8a /* #? or * */
#define MP_SET		0x8b /* _[_ad-g] */
#define MP_NOT_SET	0x8c /* _[~_ad-g] */
#define MP_DASH 	0x8d /* [ad_-g_] */
#define MP_SET_END	0x8e /* [ad-g_]_ */

#endif
