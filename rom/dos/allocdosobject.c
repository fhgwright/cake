/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <exec/memory.h>
#include <proto/exec.h>
#include <dos/exall.h>
#include <utility/tagitem.h>
#include <proto/utility.h>
#include <dos/rdargs.h>
#include <dos/dostags.h>
#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH2(APTR, AllocDosObject,

/*  SYNOPSIS */
	AROS_LHA(ULONG,            type, D1),
	AROS_LHA(struct TagItem *, tags, D2),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 38, Dos)

/*  FUNCTION
	Creates a new dos object of a given type.

    INPUTS
	type - object type.
	tags - Pointer to taglist array with additional information.

    RESULT
	Pointer to new object or NULL.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)

    switch(type)
    {
    case DOS_FILEHANDLE:
	return AllocMem(sizeof(struct FileHandle),MEMF_CLEAR);
    case DOS_FIB:
	return AllocMem(sizeof(struct FileInfoBlock),MEMF_CLEAR);
    case DOS_EXALLCONTROL:
	return AllocMem(sizeof(struct ExAllControl),MEMF_CLEAR);
    case DOS_CLI:
    {
	struct CommandLineInterface *cli = NULL;
	struct TagItem defaults[] =
	{
	/* 0 */ { ADO_DirLen,		255 },
	/* 1 */ { ADO_CommNameLen,	255 },
	/* 2 */ { ADO_CommFileLen,	255 },
	/* 3 */ { ADO_PromptLen,	255 },
		{ TAG_END, 0 }
    	};
	STRPTR dir = NULL, command = NULL, file = NULL, prompt = NULL;
	/* C has no exceptions. This is a simple replacement. */
#define ENOMEM_IF(a) if(a) goto enomem /* Throw out of memory. */	

	cli = AllocMem(sizeof(struct CommandLineInterface),MEMF_CLEAR);
	ENOMEM_IF(cli == NULL);

	cli->cli_FailLevel = 10;
	cli->cli_Background = DOSTRUE;
	ApplyTagChanges(defaults,tags);

	dir = AllocVec(defaults[0].ti_Data+1,MEMF_PUBLIC|MEMF_CLEAR);
	ENOMEM_IF(dir == NULL);
	cli->cli_SetName = MKBADDR(dir);

	command = AllocVec(defaults[1].ti_Data+1,MEMF_PUBLIC|MEMF_CLEAR);
	ENOMEM_IF(command == NULL);
	cli->cli_CommandName = MKBADDR(command);

	file = AllocVec(defaults[2].ti_Data+1,MEMF_PUBLIC|MEMF_CLEAR);
	ENOMEM_IF(file == NULL);
	cli->cli_CommandFile = MKBADDR(file);

	prompt = AllocVec(defaults[3].ti_Data+1,MEMF_PUBLIC|MEMF_CLEAR);
	ENOMEM_IF(prompt == NULL);
	cli->cli_Prompt = MKBADDR(prompt);

	return cli;

enomem:
	if (cli)
	    FreeMem(cli,sizeof(struct CommandLineInterface));

	if (dir)
	    FreeVec(dir);

	if (command)
	    FreeVec(command);

	if (file)
	    FreeVec(file);

	if (prompt)
	    FreeVec(prompt);

	return NULL;
    }
    case DOS_RDARGS:
	return AllocVec(sizeof(struct RDArgs),MEMF_CLEAR);
    }
    return NULL;
    AROS_LIBFUNC_EXIT
} /* AllocDosObject */
