/*
    (C) 1999 AROS - The Amiga Research OS
    $Id$

    Desc: 
    Lang: English
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <dos/dosextens.h>
#include <libraries/iffparse.h>
#include <utility/name.h>
#include <workbench/startup.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/iffparse.h>
#include <proto/alib.h>

#include <string.h>

#include "../libs/datatypes/datatypes_intern.h"
#undef DOSBase
#undef UtilityBase
#undef IFFParseBase
#undef LocaleBase
#undef DTList


/******************************** PROTOTYPES *********************************/

struct StackVars;		/* forward declaration */

BOOL DateScan(struct StackVars *sv);
void ScanDirectory(struct StackVars *sv, STRPTR pattern);
struct DataTypesList *CreateDTList(struct StackVars *sv);
struct CompoundDatatype *CreateBasicType(struct StackVars *sv,
					 struct List *list,
					 struct List *globallist, STRPTR name,
					 UWORD Flags, ULONG GroupID, ULONG ID);
void LoadDatatype(struct StackVars *sv, STRPTR name);
struct CompoundDatatype *CreateDatatype(struct StackVars *sv,
					struct IFFHandle *iff);
struct CompoundDatatype *AddDatatype(struct StackVars *sv, 
				     struct CompoundDatatype *cdt);
void DeleteDatatype(struct StackVars *sv, struct CompoundDatatype *cdt);
void AlphaInsert(struct StackVars *sv, struct List *list, struct Node *node);
void PrioInsert(struct StackVars *sv, struct List *list,
		struct CompoundDatatype *cdt);
struct Node *__FindNameNoCase(struct StackVars *sv, struct List *list,
			      STRPTR name);
LONG ReadFunc(struct StackVars *sv, UBYTE *buffer, ULONG length);
UBYTE *AllocFunc(ULONG size, ULONG flags);
void FreeFunc(UBYTE *memory, ULONG size);
struct NamedObject *allocnamedobject(struct StackVars *sv, STRPTR name, 
				     Tag FirstTag, ...);


/********************************* CONSTANTS *********************************/

UBYTE Version[]="$VER: AddDatatypes 41.0";

#define EXCL_LEN 18
UBYTE ExcludePattern[] = "#?.(info|backdrop)";

UBYTE Template[] = "FILES/M,QUIET/S,REFRESH/S";

struct ArgArray
{
    UBYTE **aa_Files;
    ULONG   aa_Quiet;
    ULONG   aa_Refresh;
};

#define  ID_DTCD  MAKE_ID('D','T','C','D')
#define  ID_DTTL  MAKE_ID('D','T','T','L')

#define  NUM_PROP  2

LONG PropArray[2*NUM_PROP]=
{
    ID_DTYP, ID_DTHD,
    ID_DTYP, ID_DTCD
};

#define  NUM_COLL  1

LONG CollArray[2*NUM_COLL]=
{
    ID_DTYP, ID_DTTL
};


LONG_FUNC FunctionArray[]=
{				/* Note! */
    (LONG_FUNC)&ReadFunc,
    (LONG_FUNC)&AllocFunc,
    (LONG_FUNC)&FreeFunc
};


struct StackVars
{
    struct Library       *DOSBase;
    struct Library       *UtilityBase;
    struct Library       *LocaleBase;
    struct Library       *IFFParseBase;
    struct Process       *MyProc;
    struct WBStartup     *WBMsg;
    struct DataTypesList *DTList;
    UBYTE                 ExclPat[2*EXCL_LEN+2+1];
    struct ArgArray  AA;
    UBYTE           *HookBuffer;
    ULONG            HookBufSize;
    ULONG            HookPosition;
};

#undef  SysBase
#define DOSBase           sv->DOSBase
#define UtilityBase       sv->UtilityBase
#define LocaleBase        sv->LocaleBase
#define IFFParseBase      sv->IFFParseBase
#define MyProc            sv->MyProc
#define WBMsg             sv->WBMsg
#define DTList            sv->DTList
#define ExclPat           sv->ExclPat
#define AA                sv->AA
#define HookBuffer        sv->HookBuffer
#define HookBufSize       sv->HookBufSize
#define HookPosition      sv->HookPosition


/****** AddDatatypes/main *****************************************************
*
*   NAME
*        main - well... main
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

LONG main(void)
{
    struct StackVars  vars;
    struct StackVars *sv;
    ULONG             result = RETURN_FAIL;
	
    memset(&vars, 0, sizeof(struct StackVars));
    sv = &vars;
    
    MyProc = (struct Process *)FindTask(NULL);

    if(!MyProc->pr_CLI)
    {
	WaitPort(&MyProc->pr_MsgPort);
	WBMsg = (struct WBStartup*)GetMsg(&MyProc->pr_MsgPort);
    }
    
    //    if((*((struct ExecBase**)4))->LibNode.lib_Version>=37)
    //    {
	if((DOSBase = OpenLibrary("dos.library", 37)))
	{
	    if((UtilityBase = OpenLibrary("utility.library", 37)))
	    {
		if((LocaleBase = OpenLibrary("locale.library", 37)))
		{
		    if((IFFParseBase = OpenLibrary("iffparse.library", 37)))
		    {
			if((DTList = CreateDTList(sv)))
			{
			    ParsePatternNoCase(ExcludePattern, ExclPat, 
					       sizeof(ExclPat));
			    
			    ObtainSemaphore(&DTList->dtl_Lock);
			    
			    if(WBMsg)
			    {
				UWORD num;
				struct WBArg *wa = &WBMsg->sm_ArgList[1];

				for(num = 1; num<WBMsg->sm_NumArgs; wa++)
				{
				    BPTR olddir = CurrentDir(wa->wa_Lock);
				    LoadDatatype(sv, wa->wa_Name);
				    CurrentDir(olddir);
				}

				result = RETURN_OK;
			    }
			    else
			    {
				struct RDArgs *RDArgs;
				
				if(!(RDArgs = ReadArgs(Template, (LONG*)&AA,
						       NULL)))
				{
				    PrintFault(IoErr(), NULL);
				}
				else
				{
				    if(AA.aa_Refresh)
				    {
					if(DateScan(sv))
					{
					    ScanDirectory(sv, "DEVS:DataTypes");
					}
				    }
				    else
				    {
					UBYTE **files = AA.aa_Files;

					if(files)
					{
					    while(*files)
					    {
						ScanDirectory(sv, *files);
						files++;
					    }
					}
				    }

				    result = RETURN_OK;
				    FreeArgs(RDArgs);
				}
			    }
			    
			    ReleaseSemaphore(&DTList->dtl_Lock);
			}

			CloseLibrary(IFFParseBase);
		    }

		    CloseLibrary(LocaleBase);
		}

		CloseLibrary(UtilityBase);
	    }

	    CloseLibrary(DOSBase);
	}

	//     }
    
    if(WBMsg)
    {
	ReplyMsg((struct Message*)WBMsg);
	Forbid();
    }
    
    return result;
}



/****** AddDatatypes/DateScan *************************************************
*
*   NAME
*        DateScan - See if datatypes descriptors need updating
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

BOOL DateScan(struct StackVars *sv)
{
    BOOL   result = TRUE;
    BPTR   lock;
    struct FileInfoBlock *fib;
    
    if((lock = Lock("DEVS:Datatypes", ACCESS_READ)))
    {
	if((fib = AllocDosObject(DOS_FIB, NULL)))
	{
	    if(Examine(lock, fib))
	    {
		if(!CompareDates(&fib->fib_Date, &DTList->dtl_DateStamp))
		{
		    result = FALSE;
		}
		else
		{
		    DTList->dtl_DateStamp = fib->fib_Date;
		}
	    }

	    FreeDosObject(DOS_FIB,fib);
	}

	UnLock(lock);
    }
    
    return result;
}



/****** AddDatatypes/ScanDirectory ********************************************
*
*   NAME
*        ScanDirectory - Scan a directory recursively for DT descriptors
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void ScanDirectory(struct StackVars *sv, STRPTR pattern)
{
    struct AnchorPath *AnchorPath;
    LONG               RetVal;
    BPTR               OldDir;
	
    if((AnchorPath = (struct AnchorPath *)AllocVec(sizeof(struct AnchorPath),
						  MEMF_CLEAR)))
    {
	AnchorPath->ap_BreakBits = SIGBREAKF_CTRL_C;
	
	RetVal = MatchFirst(pattern, AnchorPath);

	while(!RetVal)
	{
	    if(CheckSignal(SIGBREAKF_CTRL_C))
	    {
		if(!AA.aa_Quiet)
		{
		    PrintFault(ERROR_BREAK, NULL);
		}

		break;
	    }
	    
	    if(AnchorPath->ap_Info.fib_DirEntryType > 0L)
	    {
		if(!(AnchorPath->ap_Flags & APF_DIDDIR))
		{
		    AnchorPath->ap_Flags |= APF_DODIR;
		}

		AnchorPath->ap_Flags &= ~APF_DIDDIR;
	    }
	    else
	    {
		if(!MatchPatternNoCase(ExclPat, 
				       AnchorPath->ap_Info.fib_FileName))
		{
		    OldDir = CurrentDir(AnchorPath->ap_Current->an_Lock);
		    
		    LoadDatatype(sv, AnchorPath->ap_Info.fib_FileName);
		    
		    CurrentDir(OldDir);
		}
	    }

	    RetVal = MatchNext(AnchorPath);
	}

	if(RetVal != ERROR_NO_MORE_ENTRIES)
	{
	    if(!AA.aa_Quiet)
	    {
		PrintFault(RetVal, NULL);
	    }
	}

	MatchEnd(AnchorPath);
	
	FreeVec((APTR)AnchorPath);
    }
}


/****** AddDatatypes/CreateDTList *********************************************
*
*   NAME
*        CreateDTList - Create and initialize the DataTypesList
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct DataTypesList *CreateDTList(struct StackVars *sv)
{
    struct DataTypesList *dtl = NULL;
    struct NamedObject   *no  = NULL;
    
    if((no = FindNamedObject(NULL, DATATYPESLIST, NULL)))
    {
	dtl = (struct DataTypesList*)no->no_Object;
    }
    
    if(!dtl)
    {
	if((no = allocnamedobject(sv, DATATYPESLIST,
				  ANO_NameSpace, TRUE,
				  ANO_UserSpace, sizeof(struct DataTypesList),
				  ANO_Flags    , NSF_NODUPS | NSF_CASE,
				  TAG_DONE)))
	    {
		if(!(dtl = (struct DataTypesList*)no->no_Object))
		{
		    FreeNamedObject(no);
		    no = NULL;
		}
	    }
	
	if(dtl)
	{
	    InitSemaphore(&dtl->dtl_Lock);
	    
	    NewList(&dtl->dtl_SortedList);
	    NewList(&dtl->dtl_BinaryList);
	    NewList(&dtl->dtl_ASCIIList);
	    NewList(&dtl->dtl_IFFList);
	    NewList(&dtl->dtl_MiscList);
	    
	    if(!AddNamedObject(NULL, no))
	    {
		FreeNamedObject(no);
		no = NULL;
		dtl = NULL;
	    }
	}
    }
    
    if(dtl)
    {
	if(!__FindNameNoCase(sv, &dtl->dtl_BinaryList, "binary"))
	{
	    CreateBasicType(sv, &dtl->dtl_BinaryList, &dtl->dtl_SortedList,
			    "binary", DTF_BINARY, 
			    MAKE_ID('b','i','n','a'), 
			    MAKE_ID('s','y','s','t'));
	}

	if(!__FindNameNoCase(sv, &dtl->dtl_ASCIIList, "ascii"))
	{
	    CreateBasicType(sv, &dtl->dtl_ASCIIList,  &dtl->dtl_SortedList,
			    "ascii", DTF_ASCII,
			    MAKE_ID('a','s','c','i'),
			    MAKE_ID('t','e','x','t'));
	}

	if(!__FindNameNoCase(sv, &dtl->dtl_IFFList, "iff"))
	{
	    CreateBasicType(sv, &dtl->dtl_IFFList, &dtl->dtl_SortedList,
			    "iff", DTF_IFF,
			    MAKE_ID('i','f','f', 0 ),
			    MAKE_ID('s','y','s','t'));
	}

	if(!__FindNameNoCase(sv, &dtl->dtl_MiscList, "directory"))
	{
	    CreateBasicType(sv, &dtl->dtl_MiscList, &dtl->dtl_SortedList,
			    "directory", DTF_MISC, 
			    MAKE_ID('d','i','r','e'), 
			    MAKE_ID('s','y','s','t'));
	}
    }
    
    if(no)
    {
	ReleaseNamedObject(no);
    }
    
    
    return dtl;
}



/****** AddDatatypes/CreateBasicType ******************************************
*
*   NAME
*        CreateBasicType - Initialize one of the basic types
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct CompoundDatatype *CreateBasicType(struct StackVars *sv, 
					 struct List *list, 
					 struct List *globallist, STRPTR name,
					 UWORD Flags, ULONG GroupID, ULONG ID)
{
    struct CompoundDatatype *cdt;
    ULONG AllocLen = sizeof(struct CompoundDatatype) + strlen(name) + 1;
    
    if((cdt = AllocVec(AllocLen, MEMF_PUBLIC | MEMF_CLEAR)))
    {
	cdt->DT.dtn_Header = &cdt->DTH;
	
	strcpy((UBYTE*)(cdt + 1), name);
	
	cdt->DTH.dth_Name=
	    cdt->DTH.dth_BaseName=
	    cdt->DT.dtn_Node1.ln_Name=
	    cdt->DT.dtn_Node2.ln_Name=(UBYTE*)(cdt + 1);
	
	cdt->DTH.dth_GroupID = GroupID;
	cdt->DTH.dth_ID = ID;
	
	cdt->DTH.dth_Flags = Flags;
		
	NewList(&cdt->DT.dtn_ToolList);
		
	cdt->DT.dtn_Length = AllocLen;
		
	AddTail(list, &cdt->DT.dtn_Node1);
		
	AlphaInsert(sv, globallist, &cdt->DT.dtn_Node2);
    }

    return cdt;
}



/****** AddDatatypes/LoadDatatype *********************************************
*
*   NAME
*        LoadDatatype - Load and install a single datatype descriptor
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void LoadDatatype(struct StackVars *sv, STRPTR name)
{
    struct IFFHandle *iff;
    
    if((iff = AllocIFF()))
    {
	if((iff->iff_Stream = (IPTR)Open(name, MODE_OLDFILE))) /* Why IPTR? */
	{
	    InitIFFasDOS(iff);
	    
	    if(!OpenIFF(iff, IFFF_READ))
	    {
		if(!PropChunks(iff, PropArray, NUM_PROP))
		{
		    if(!CollectionChunks(iff, CollArray, NUM_COLL))
		    {
			if(!StopOnExit(iff, ID_DTYP, ID_FORM))
			{
			    while(ParseIFF(iff, IFFPARSE_SCAN) == IFFERR_EOC)
			    {
				CreateDatatype(sv, iff);
			    }
			}
		    }
		}
		
		CloseIFF(iff);
	    }

	    Close((BPTR)iff->iff_Stream);
	}

	FreeIFF(iff);
    }
    else
    {
	SetIoErr(ERROR_NO_FREE_STORE);
    }
}



/****** AddDatatypes/CreateDatatype *******************************************
*
*   NAME
*        CreateDatatype - create a datatype from IFF chunks
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct CompoundDatatype *CreateDatatype(struct StackVars *sv,
					struct IFFHandle *iff)
{
    struct CompoundDatatype *cdt = NULL;
    struct StoredProperty *prop;
    ULONG  AllocLen;
    UBYTE *func;
    LONG   DefaultStack = 4096;
    BPTR   SegList;
    
    if((prop = FindProp(iff, ID_DTYP, ID_DTHD)))
    {
	AllocLen = sizeof(struct CompoundDatatype) - 
	    sizeof(struct DataTypeHeader) + prop->sp_Size;
	
	if(!(cdt = AllocVec(AllocLen, MEMF_PUBLIC | MEMF_CLEAR)))
	{
	    SetIoErr(ERROR_NO_FREE_STORE);
	}
	else
	{
	    cdt->DT.dtn_Header= &cdt->DTH;
	    
	    CopyMem(prop->sp_Data, &cdt->DTH, prop->sp_Size);
	    
	    cdt->DTH.dth_Name = (STRPTR)
		((ULONG)cdt->DTH.dth_Name    +(ULONG)&cdt->DTH);

	    cdt->DTH.dth_BaseName = (STRPTR)
		((ULONG)cdt->DTH.dth_BaseName+(ULONG)&cdt->DTH);

	    cdt->DTH.dth_Pattern = (STRPTR)
		((ULONG)cdt->DTH.dth_Pattern +(ULONG)&cdt->DTH);

	    cdt->DTH.dth_Mask = (WORD*)
		((ULONG)cdt->DTH.dth_Mask    +(ULONG)&cdt->DTH);
	    
	    cdt->DT.dtn_Node1.ln_Name =
		cdt->DT.dtn_Node2.ln_Name = cdt->DTH.dth_Name;
			
	    NewList(&cdt->DT.dtn_ToolList);
	    
	    cdt->DT.dtn_Length = AllocLen;
	    
	    if((prop = FindProp(iff, ID_DTYP, ID_DTCD)))
	    {
		if((func = AllocVec(prop->sp_Size, MEMF_PUBLIC | MEMF_CLEAR)))
		{
		    cdt->DTCDChunk = func;
		    cdt->DTCDSize = prop->sp_Size;
		    
		    CopyMem(prop->sp_Data,func,prop->sp_Size);
		    
		    HookBuffer = cdt->DTCDChunk;
		    HookBufSize = cdt->DTCDSize;
		    HookPosition = 0;

		    if((SegList = InternalLoadSeg((BPTR)sv, NULL,
						  (LONG_FUNC)FunctionArray,
						  &DefaultStack)))
		    {
			cdt->SegList = SegList;
			cdt->Function = (APTR)((((ULONG)SegList) << 2) + 4);
		    }
		}
	    }

	    cdt = AddDatatype(sv, cdt);
	}
    }
    
    return cdt;
}



/****** AddDatatypes/AddDatatype **********************************************
*
*   NAME
*        AddDatatype - add a datatype to the system
*
*   SYNOPSIS
*
*   FUNCTION
*
*        This subroutine tries to add a datatype to the system datatypes
*        list. If the datatype already exists, it will be replaced or
*        updated. In case of an error, the CompoundDatatype will be deleted
*        and a NULL pointer is returned.
*
*        The CompoundDatatype pointer you passed in will be invalid after
*        calling this function. Use the returned handle instead.
*        DO NOT USE THE OLD POINTER IN ANY MORE!
*
*   INPUTS
*
*   RETURNS
*
*        A pointer to a CompoundDatatype in the system datatypes list
*        or a NULL pointer for failure
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct CompoundDatatype *AddDatatype(struct StackVars *sv,
				     struct CompoundDatatype *cdt)
{
    struct List *typelist;
    BOOL   Success = FALSE;
    ULONG  AllocSize;
    ULONG  result;
    struct CompoundDatatype *oldcdt;
    
    switch(cdt->DTH.dth_Flags & DTF_TYPE_MASK)
    {
    case DTF_BINARY: typelist= &DTList->dtl_BinaryList; break;
    case DTF_ASCII:  typelist= &DTList->dtl_ASCIIList;  break;
    case DTF_IFF:    typelist= &DTList->dtl_IFFList;    break;
    case DTF_MISC:   typelist= &DTList->dtl_MiscList;   break;
    default:         typelist= NULL;
    }
    
    if(typelist)
    {
	cdt->DT.dtn_Node1.ln_Name = cdt->DT.dtn_Node2.ln_Name = cdt->DTH.dth_Name;
	
	Success = TRUE;
	
	if((!Stricmp(cdt->DTH.dth_Pattern, "#?")) || 
	   (!strlen(cdt->DTH.dth_Pattern)) )
	{
	    cdt->FlagLong |= CFLGF_PATTERN_UNUSED;
	}
	else
	{
	    cdt->FlagLong &= ~(CFLGF_PATTERN_UNUSED);
	    
	    AllocSize = 2*strlen(cdt->DTH.dth_Pattern) + 2;

	    if(!(cdt->ParsePatMem = AllocVec(AllocSize,
					     MEMF_PUBLIC | MEMF_CLEAR)))
	    {
		Success = FALSE;
	    }
	    else
	    {
		cdt->ParsePatSize = AllocSize;
		
		result = ParsePatternNoCase(cdt->DTH.dth_Pattern,
					    cdt->ParsePatMem, AllocSize);
		
		if(result == 1)
		{
		    cdt->FlagLong |= CFLGF_IS_WILD;
		}
		else
		{
		    FreeVec(cdt->ParsePatMem);
		    cdt->ParsePatMem = NULL;
		    cdt->ParsePatSize = 0;
		    
		    if(result == 0)
		    {
			cdt->FlagLong &= ~(CFLGF_IS_WILD);
		    }
		    else
		    {
			Success = FALSE;
		    }
		}
	    }
	}

	if (Success)
	{
	    if((oldcdt = (struct CompoundDatatype*)__FindNameNoCase(sv,
								    typelist,
								    cdt->DT.dtn_Node1.ln_Name)))
	    {
		if (oldcdt->OpenCount)
		{
		    Success = FALSE;
		}
		else
		{
		    if((Stricmp(oldcdt->DTH.dth_Name, cdt->DTH.dth_Name)) ||
		       (Stricmp(oldcdt->DTH.dth_BaseName, cdt->DTH.dth_BaseName)) ||
		       (Stricmp(oldcdt->DTH.dth_Pattern, cdt->DTH.dth_Pattern)) ||
		       (oldcdt->DTH.dth_Flags != cdt->DTH.dth_Flags) ||
		       (oldcdt->DTH.dth_Priority != cdt->DTH.dth_Priority) ||
		       (oldcdt->DTH.dth_MaskLen != cdt->DTH.dth_MaskLen))
		    {
			DeleteDatatype(sv, oldcdt);
			oldcdt = NULL;
		    }
		    else
		    {
			oldcdt->DTH.dth_GroupID = cdt->DTH.dth_GroupID;
			oldcdt->DTH.dth_ID      = cdt->DTH.dth_ID;
			CopyMem(cdt->DTH.dth_Mask,cdt->DTH.dth_Mask,
				(ULONG)(sizeof(WORD)*cdt->DTH.dth_MaskLen));
		    }
		}
	    }
	    
	    if(Success)
	    {
		if(oldcdt)
		{
		    DeleteDatatype(sv, cdt);
		    cdt = oldcdt;
		}
		else
		{
		    if(cdt->DT.dtn_FunctionName)
		    {
			LONG DefaultStack = 4096;
			BPTR file;
			ULONG AllocLen;
			BPTR SegList;
			
			if((file = Open(cdt->DT.dtn_FunctionName, MODE_OLDFILE)))
			{
			    if(Seek(file, 0, OFFSET_END) >= 0)
			    {
				if((AllocLen = Seek(file, 0,
						    OFFSET_BEGINNING)) > 0)
				{
				    if((cdt->DTCDChunk = AllocVec(AllocLen,
								  MEMF_PUBLIC | MEMF_CLEAR)))
				    {
					cdt->DTCDSize = AllocLen;

					if(Read(file, cdt->DTCDChunk, AllocLen) == AllocLen)
					{
					    HookBuffer = cdt->DTCDChunk;
					    HookBufSize = cdt->DTCDSize;
					    HookPosition = 0;

					    if((SegList = InternalLoadSeg((BPTR)sv, NULL, (LONG_FUNC)FunctionArray, &DefaultStack)))
					    {
						cdt->SegList = SegList;
						cdt->Function = (APTR)((((ULONG)SegList)<<2)+4);
					    }
					}
					else
					{
					    FreeVec(cdt->DTCDChunk);
					    cdt->DTCDChunk = NULL;
					    cdt->DTCDSize = 0;
					}
				    }
				}
			    }
			}
			cdt->DT.dtn_FunctionName=NULL;
		    }
		    
		    if(cdt->DTH.dth_MaskLen > DTList->dtl_LongestMask)
		    {
			DTList->dtl_LongestMask = cdt->DTH.dth_MaskLen;
		    }
		    
		    PrioInsert(sv, typelist, cdt);
		    
		    AlphaInsert(sv, &DTList->dtl_SortedList, &cdt->DT.dtn_Node2);
		}
	    }
	}
    }
    
    if(!Success)
    {
	DeleteDatatype(sv, cdt);
	cdt = NULL;
    }
    
    return cdt;
}



/****** AddDatatypes/DeleteDatatype *******************************************
*
*   NAME
*        DeleteDatatype - unlink and deallocate a CompoundDatatype structure
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void DeleteDatatype(struct StackVars *sv, struct CompoundDatatype *cdt)
{
    if(cdt)
    {
	if(cdt->ParsePatMem)
	{
	    FreeVec(cdt->ParsePatMem);
	    cdt->ParsePatMem = NULL;
	    cdt->ParsePatSize = 0;
	}
	
	if(cdt->DTCDChunk)
	{
	    FreeVec(cdt->DTCDChunk);
	    cdt->DTCDChunk = NULL;
	    cdt->DTCDSize = 0;
	}
		
	if(cdt->SegList)
	{
	    UnLoadSeg(cdt->SegList);
	    cdt->SegList = NULL;
	    cdt->Function = NULL;
	}
		
	if(cdt->DT.dtn_Node1.ln_Succ && cdt->DT.dtn_Node1.ln_Pred)
	{
	    Remove(&cdt->DT.dtn_Node1);
	    Remove(&cdt->DT.dtn_Node2);
	    cdt->DT.dtn_Node1.ln_Succ = cdt->DT.dtn_Node1.ln_Pred =
		cdt->DT.dtn_Node2.ln_Succ = cdt->DT.dtn_Node2.ln_Pred = NULL;
	}

	FreeVec(cdt);
    }
}



/****** AddDatatypes/AlphaInsert **********************************************
*
*   NAME
*        AlphaInsert - enqueue a node alphabetically into a list 
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void AlphaInsert(struct StackVars *sv, struct List *list, struct Node *node)
{
    struct Node *cur,*prev=NULL;
    
    for(cur = list->lh_Head; cur->ln_Succ; prev = cur, cur = cur->ln_Succ)
    {
	if(Stricmp(cur->ln_Name, node->ln_Name) > 0)
	    break;
    }

    Insert(list, node, prev);
}



/****** AddDatatypes/PrioInsert **********************************************
*
*   NAME
*        PrioInsert - enqueue a CompoundDatatype correctly in the type list
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void PrioInsert(struct StackVars *sv, struct List *list,
		struct CompoundDatatype *cdt)
{
    struct CompoundDatatype *cur, *prev = NULL;
    WORD diff;
    
    for(cur = (struct CompoundDatatype*)list->lh_Head;
	cur->DT.dtn_Node1.ln_Succ;
	prev = cur, cur = (struct CompoundDatatype*)cur->DT.dtn_Node1.ln_Succ)
    {
	diff = (cdt->Function ? 1 : 0) - (cur->Function ? 1 : 0);
	
	if(diff > 0)
	    break;
	
	if(!diff)
	{
	    UWORD MinMask = (cdt->DTH.dth_MaskLen < cur->DTH.dth_MaskLen) ?
		cdt->DTH.dth_MaskLen : cur->DTH.dth_MaskLen;
	    WORD *cdtmask = cdt->DTH.dth_Mask;
	    WORD *curmask=cur->DTH.dth_Mask;

	    while(!diff && MinMask--)
		diff= *(curmask++) - *(cdtmask++);
			
	    if(diff > 0)
		break;
	    
	    if(!diff)
	    {
		diff = cdt->DTH.dth_MaskLen - cur->DTH.dth_MaskLen;
		
		if(diff > 0)
		    break;
		
		if(!diff)
		{
		    diff = (((cdt->FlagLong & CFLGF_PATTERN_UNUSED) || cdt->DTH.dth_Pattern==NULL) ? 0 : 1) -
			(((cur->FlagLong & CFLGF_PATTERN_UNUSED) || cur->DTH.dth_Pattern==NULL) ? 0 : 1);
		    
		    if(diff > 0)
			break;
		    
		    if(!diff)
		    {
			diff = cdt->DTH.dth_Priority - cur->DTH.dth_Priority;
			
			if(diff > 0)
			    break;
		    }
		}
	    }
	}
    }

    Insert(list, &cdt->DT.dtn_Node1, (struct Node *)prev);
}



/****** AddDatatypes/__FindNameNoCase *****************************************
*
*   NAME
*        __FindNameNoCase - find a node in a list (case insensitive)
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

struct Node *__FindNameNoCase(struct StackVars *sv, struct List *list,
			      STRPTR name)
{
    struct Node *node;
    struct Node *result = NULL;
    
    for(node = list->lh_Head; node->ln_Succ; node = node->ln_Succ)
    {
	if(!Stricmp(node->ln_Name, name))
	{
	    result = node;
	    break;
	}
    }

    return result;
}



/****** AddDatatypes/ReadFunc *************************************************
*
*   NAME
*        ReadFunc - data read hook for InternalLoadSeg
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

LONG ReadFunc(struct StackVars *sv, UBYTE *buffer, ULONG length)
{
    LONG maxlen = HookBufSize-HookPosition;
    LONG actual = length > maxlen ? maxlen : length;
    
    CopyMem(HookBuffer+HookPosition, buffer, actual);
	
    HookPosition += actual;
    
    return actual;
}



/****** AddDatatypes/AllocFunc ************************************************
*
*   NAME
*        AllocFunc - memory allocation hook for InternalLoadSeg
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

UBYTE *AllocFunc(ULONG size, ULONG flags)
{
    return(AllocMem(size, flags));
}



/****** AddDatatypes/FreeFunc *************************************************
*
*   NAME
*        FreeFunc - memory freeing hook for InternalLoadSeg
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RETURNS
*
*   EXAMPLE
*
*   SEE ALSO
*
******************************************************************************
*
*/

void FreeFunc(UBYTE *memory, ULONG size)
{
    FreeMem(memory, size);
}



/******************************* STUB ROUTINES ********************************/

struct NamedObject *allocnamedobject(struct StackVars *sv, STRPTR name,
				     Tag FirstTag, ...)
{
    return AllocNamedObjectA(name, (struct TagItem*)&FirstTag);
}
