/* 
   $Id$
*/

#ifndef DEBUG
#define DEBUG 1
#endif

#include <proto/exec.h>

#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/types.h>
#include <dos/dos.h>

#include <aros/libcall.h>
#include <aros/debug.h>

#include "afshandler.h"
#include "volumes.h"

#define NEWLIST(l)                          \
((l)->lh_Head=(struct Node *)&(l)->lh_Tail, \
 (l)->lh_Tail=NULL,                         \
 (l)->lh_TailPred=(struct Node *)(l))

extern const char name[];
extern const char version[];
extern const APTR inittab[4];
extern void *const afsfunctable[];
extern const UBYTE afsdatatable;
extern struct afsbase *AROS_SLIB_ENTRY(init,afsdev)();
extern void AROS_SLIB_ENTRY(open,afsdev)();
extern BPTR AROS_SLIB_ENTRY(close,afsdev)();
extern BPTR AROS_SLIB_ENTRY(expunge,afsdev)();
extern int AROS_SLIB_ENTRY(null,afsdev)();
extern void AROS_SLIB_ENTRY(beginio,afsdev)();
extern LONG AROS_SLIB_ENTRY(abortio,afsdev)();
extern void work();
extern const char afshandlerend;

int AFS_entry(void)
{
	/* If the device was executed by accident return error code. */
	return -1;
}

const struct Resident AFS_resident=
{
	RTC_MATCHWORD,
	(struct Resident *)&AFS_resident,
	(APTR)&afshandlerend,
	RTF_COLDSTART | RTF_AFTERDOS | RTF_AUTOINIT,
	41,
	NT_DEVICE,
	-122,
	(char *)name,
	(char *)&version[6],
	(ULONG *)inittab
};

static const char name[]="afs.handler";
static const char version[]="$VER: afs-handler 41.0 (2001-01-17)\n";

static const APTR inittab[4]=
{
	(APTR)sizeof(struct afsbase),
	(APTR)afsfunctable,
	(APTR)&afsdatatable,
	&AROS_SLIB_ENTRY(init,afsdev)
};

void *const afsfunctable[]=
{
	&AROS_SLIB_ENTRY(open,afsdev),
	&AROS_SLIB_ENTRY(close,afsdev),
	&AROS_SLIB_ENTRY(expunge,afsdev),
	&AROS_SLIB_ENTRY(null,afsdev),
	&AROS_SLIB_ENTRY(beginio,afsdev),
	&AROS_SLIB_ENTRY(abortio,afsdev),
	(void *)-1
};

const UBYTE afsdatatable = 0;

AROS_LH2(struct afsbase *, init,
 AROS_LHA(struct afsbase *, afsbase, D0),
 AROS_LHA(BPTR,             segList, A0),
      struct ExecBase *, SysBase, 0, afsdev)
{
	AROS_LIBFUNC_INIT

	struct Task *task;
	APTR stack;

	afsbase->seglist = segList;
	afsbase->sysbase = SysBase;
	afsbase->dosbase = (struct DosLibrary *)OpenLibrary("dos.library",39);
	if (afsbase->dosbase != NULL)
	{
		afsbase->intuitionbase = (struct IntuitionBase *)OpenLibrary("intuition.library",39);
		if (afsbase->intuitionbase)
		{
			NEWLIST(&afsbase->port.mp_MsgList);
			afsbase->port.mp_Node.ln_Type = NT_MSGPORT;
			afsbase->port.mp_SigBit = SIGBREAKB_CTRL_F;
			NEWLIST(&afsbase->rport.mp_MsgList);
			afsbase->rport.mp_Node.ln_Type = NT_MSGPORT;
			afsbase->rport.mp_Flags = PA_SIGNAL;
			afsbase->rport.mp_SigBit = SIGB_SINGLE;
			task = (struct Task *)AllocMem(sizeof(struct Task), MEMF_PUBLIC | MEMF_CLEAR);
			if (task != NULL)
			{
				afsbase->port.mp_SigTask = task;
				afsbase->port.mp_Flags = PA_IGNORE;
				NEWLIST(&task->tc_MemEntry);
				task->tc_Node.ln_Type = NT_TASK;
				task->tc_Node.ln_Name = "afs.handler task";
				stack = AllocMem(AROS_STACKSIZE, MEMF_PUBLIC);
				if (stack != NULL)
				{
					task->tc_SPLower = stack;
					task->tc_SPUpper = (BYTE *)stack+AROS_STACKSIZE;
#if AROS_STACK_GROWS_DOWNWARDS
					task->tc_SPReg = (BYTE *)task->tc_SPUpper-SP_OFFSET-sizeof(APTR);
					((APTR *)task->tc_SPUpper)[-1] = afsbase;
#else
					task->tc_SPReg = (BYTE *)task->tc_SPLower-SP_OFFSET+sizeof(APTR);
					*(APTR *)task->tc_SPLower = afsbase;
#endif
					if (AddTask(task,work,NULL) != NULL)
						return afsbase;
					FreeMem(stack, AROS_STACKSIZE);
				}
				FreeMem(task, sizeof(struct Task));
			}
			CloseLibrary((struct Library *)afsbase->intuitionbase);
		}
		CloseLibrary((struct Library *)afsbase->dosbase);
	}
	return NULL;
	AROS_LIBFUNC_EXIT
}

#include "baseredef.h"

AROS_LH3(void, open,
 AROS_LHA(struct IOFileSys *, iofs, A1),
 AROS_LHA(ULONG,              unitnum, D0),
 AROS_LHA(ULONG,              flags, D1),
           struct afsbase *, afsbase, 1,afsdev)
{
	AROS_LIBFUNC_INIT
	struct Volume *volume;

	unitnum = flags = 0;
	afsbase->device.dd_Library.lib_OpenCnt++;
	afsbase->rport.mp_SigTask=FindTask(NULL);
	volume = initVolume
		(
			afsbase,
			iofs->IOFS.io_Device,
			iofs->io_Union.io_OpenDevice.io_DeviceName,
			iofs->io_Union.io_OpenDevice.io_Unit,
			(struct DosEnvec *)iofs->io_Union.io_OpenDevice.io_Environ,
			&iofs->io_DosError
		);
	if (volume)
	{
		iofs->IOFS.io_Unit = (struct Unit *)(&volume->ah);
		iofs->IOFS.io_Device = &afsbase->device;
		afsbase->device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
		iofs->IOFS.io_Error = 0;

		return;
	}


/*	iofs->IOFS.io_Command = -1;
	PutMsg(&afsbase->port, &iofs->IOFS.io_Message);
	WaitPort(&afsbase->rport);
	(void)GetMsg(&afsbase->rport);
	if (iofs->io_DosError == NULL)
	{
		iofs->IOFS.io_Device = &afsbase->device;
		afsbase->device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
		iofs->IOFS.io_Error = 0;
		return;
	}*/
	afsbase->device.dd_Library.lib_OpenCnt--;
	iofs->IOFS.io_Error = IOERR_OPENFAIL;
	AROS_LIBFUNC_EXIT	
}

AROS_LH0(BPTR, expunge, struct afsbase *, afsbase, 3, afsdev)
{
	AROS_LIBFUNC_INIT

	BPTR retval;

	if (afsbase->device.dd_Library.lib_OpenCnt) {
		afsbase->device.dd_Library.lib_Flags |= LIBF_DELEXP;
		return 0;
	}
	RemTask(afsbase->port.mp_SigTask);
	FreeMem(((struct Task *)afsbase->port.mp_SigTask)->tc_SPLower,AROS_STACKSIZE);
	FreeMem(afsbase->port.mp_SigTask, sizeof(struct Task));
	CloseLibrary((struct Library *)IntuitionBase);
	CloseLibrary((struct Library *)DOSBase);
	Remove(&afsbase->device.dd_Library.lib_Node);
	retval=afsbase->seglist;
	FreeMem((char *)afsbase-afsbase->device.dd_Library.lib_NegSize,
				afsbase->device.dd_Library.lib_NegSize+afsbase->device.dd_Library.lib_PosSize);
	return retval;

	AROS_LIBFUNC_EXIT
}

AROS_LH1(BPTR, close,
 AROS_LHA(struct IOFileSys *, iofs, A1),
      struct afsbase *, afsbase, 2, afsdev)
{
	AROS_LIBFUNC_INIT
	struct Volume *volume;

	afsbase->rport.mp_SigTask=FindTask(NULL);
/*	iofs->IOFS.io_Command = -2;
	PutMsg(&afsbase->port, &iofs->IOFS.io_Message);
	WaitPort(&afsbase->rport);
	(void)GetMsg(&afsbase->rport);
	if (iofs->io_DosError)
		return 0;				// there is still something to do on this volume
*/
	volume = ((struct AfsHandle *)iofs->IOFS.io_Unit)->volume;
	if (!volume->locklist)
	{
		uninitVolume(afsbase, volume);
		iofs->IOFS.io_Device=(struct Device *)-1;
		if (!--afsbase->device.dd_Library.lib_OpenCnt)
		{
			/* Delayed expunge pending? */
			if (afsbase->device.dd_Library.lib_Flags & LIBF_DELEXP)
			{
				/* Then expunge the device */
				return expunge();
			}
		}
	}
	else
	{
		iofs->IOFS.io_Error = ERROR_OBJECT_IN_USE;
	}
	return 0;
	AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null, struct afsbase *, afsbase, 4, afsdev)
{
	AROS_LIBFUNC_INIT
	return 0;
	AROS_LIBFUNC_EXIT
}

AROS_LH1(void, beginio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
           struct afsbase *, afsbase, 5, afsdev)
{
	AROS_LIBFUNC_INIT
	/* WaitIO will look into this */
    iofs->IOFS.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	/* Nothing is done quick */
    iofs->IOFS.io_Flags &= ~IOF_QUICK;
	/* So let the device task do it */
    PutMsg(&afsbase->port, &iofs->IOFS.io_Message);
	AROS_LIBFUNC_EXIT
}

AROS_LH1(LONG, abortio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
           struct afsbase *, afsbase, 6, afsdev)
{
	AROS_LIBFUNC_INIT
	return 0;
	AROS_LIBFUNC_EXIT
}

static const char afshandlerend = 0;
