#ifndef AROS_MACHINE_H
#define AROS_MACHINE_H

/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    NOTE: This file must compile *without* any other header !

    Desc: machine.h include file for Amiga/m68k
    Lang: english
*/

/* Information generated by machine.c */
#define AROS_STACK_GROWS_DOWNWARDS 1 /* Stack direction */
#define AROS_BIG_ENDIAN            1 /* Big or little endian */
#define AROS_SIZEOFULONG           4 /* Size of an ULONG */
#define AROS_WORDALIGN             2 /* Alignment for WORD */
#define AROS_LONGALIGN             2 /* Alignment for LONG */
#define AROS_PTRALIGN              2 /* Alignment for PTR */
#define AROS_IPTRALIGN             2 /* Alignment for IPTR */
#define AROS_DOUBLEALIGN           2 /* Alignment for double */
#define AROS_WORSTALIGN            8 /* Worst case alignment */

#define AROS_GET_SYSBASE	extern struct ExecBase * SysBase;
#define AROS_GET_DOSBASE        extern struct DosLibrary * DOSBase;

/* do we need a function attribute to get parameters on the stack? */
#define __stackparm

/*
    How much do I have to add to sp to get the address of the first
    byte on the stack?
*/
#define SP_OFFSET 0

/*
    One entry in a libraries' jumptable. For assembler compatibility, the
    field jmp should contain the code for an absolute jmp to a 32bit
    address. There are also a couple of macros which you should use to
    access the vector table from C.
*/
struct JumpVec
{
    unsigned char vec[4];
};

/* Any jump to an unimplemented vector will cause an access to this address */
#define _aros_empty_vector		0xc0edbabe

/* Internal macros */
#define __AROS_SET_VEC(v,a)             (*(ULONG*)(v)->vec=(ULONG)(a))
#define __AROS_GET_VEC(v)               ((APTR)(*(ULONG*)(v)->vec))

/* Use these to acces a vector table */
#define LIB_VECTSIZE			(sizeof (struct JumpVec))
#define __AROS_GETJUMPVEC(lib,n)        ((struct JumpVec *)(((UBYTE *)lib)-(n*LIB_VECTSIZE)))
#define __AROS_GETVECADDR(lib,n)        (__AROS_GET_VEC(__AROS_GETJUMPVEC(lib,n)))
#define __AROS_SETVECADDR(lib,n,addr)   (__AROS_SET_VEC(__AROS_GETJUMPVEC(lib,n),(APTR)(addr)))
#define __AROS_INITVEC(lib,n)           __AROS_SETVECADDR(lib,n,_aros_not_implemented)

/*
   We want to activate the execstubs and preserve all registers
   when calling obtainsemaphore, obtainsemaphoreshared, releasesemaphore,
   getcc, permit, forbid, enable, disable
*/
#undef UseExecstubs
#define UseExecstubs 1

/* Macros to test/set failure of AllocEntry() */
#define AROS_ALLOCENTRY_FAILED(memType) \
	((struct MemList *)((IPTR)(memType) | 0x80ul<<(sizeof(APTR)-1)*8))
#define AROS_CHECK_ALLOCENTRY(memList) \
	(!((IPTR)(memList) & 0x80ul<<(sizeof(APTR)-1)*8))

/*
    Find the next valid alignment for a structure if the next x bytes must
    be skipped.
*/
#define AROS_ALIGN(x)        (((x)+AROS_WORSTALIGN-1)&-AROS_WORSTALIGN)

/* Prototypes */
extern void _aros_not_implemented ();
extern void aros_not_implemented ();

/* How much stack do we need ? Lots :-) */
#define AROS_STACKSIZE	100000

/* RawDoFmt hook */
#define RDFCALL(hook,data,dptr) ((void(*)(UBYTE __d0,APTR __a3))(hook))(data,dptr);

/* What to do with the library base in header, prototype and call */
#define __AROS_LH_BASE(basetype,basename)   basetype basename __asm("r2")
#define __AROS_LP_BASE(basetype,basename)   void * __asm("r2")
#define __AROS_LC_BASE(basetype,basename)   basename
#define __AROS_LD_BASE(basetype,basename)   basetype __asm("r2")

/* How to transform an argument in header, prototype and call */
#define __AROS_LHA(type,name,reg)     type name
#define __AROS_LPA(type,name,reg)     type
#define __AROS_LCA(type,name,reg)     name
#define __AROS_LDA(type,name,reg)     type
#define __AROS_UFHA(type,name,reg)    type name
#define __AROS_UFPA(type,name,reg)    type
#define __AROS_UFCA(type,name,reg)    name
#define __AROS_UFDA(type,name,reg)    type

/* Prefix for library function in header, prototype and call */
#define __AROS_LH_PREFIX    /* eps */
#define __AROS_LP_PREFIX    /* eps */
#define __AROS_LC_PREFIX    /* eps */
#define __AROS_LD_PREFIX    /* eps */
#define __AROS_UFH_PREFIX   /* eps */
#define __AROS_UFP_PREFIX   /* eps */
#define __AROS_UFC_PREFIX   /* eps */
#define __AROS_UFD_PREFIX   /* eps */

/* if this is defined, all AROS_LP*-macros will expand to nothing. */
#define __AROS_USE_MACROS_FOR_LIBCALL

#endif /* AROS_MACHINE_H */

