#ifndef DOS_DOSTAGS_H
#define DOS_DOSTAGS_H

/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: Tags for DOS routines
    Lang: english
*/
#ifndef UTILITY_TAGITEM_H
#   include <utility/tagitem.h>
#endif

/**********************************************************************
 ****************************** Processes *****************************
 **********************************************************************/

/* Tags for CreateNewProc(). All tags, where no default is stated, the default
   is inherited from the parent process. Additionally you may use tags for
   AllocDosObject(DOS_CLI, ...). */
#if (AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
#   define NP_Dummy        (TAG_USER + 1000)
#else
#   define NP_Dummy	   (DOS_TAGBASE + 1000)
#endif
/* Exactly one of NP_Seglist or NP_Entry must be specified. */
  /* (BPTR) Seglist of code for process. */
#define NP_Seglist	(NP_Dummy + 1)
  /* (BOOL) Free seglist on exit? (Default: TRUE) */
#define NP_FreeSeglist	(NP_Dummy + 2)
  /* (APTR) Entry point for process code. */
#define NP_Entry	(NP_Dummy + 3)

  /* (BPTR/struct FileHandle *) Input filehandle. (Default: NIL:) */
#define NP_Input	(NP_Dummy + 4)
  /* (BPTR/struct FileHandle *) Output filehandle. (Default: NIL:) */
#define NP_Output	(NP_Dummy + 5)
  /* (BOOL) Close input filehandle on exit? (Default: TRUE) */
#define NP_CloseInput	(NP_Dummy + 6)
  /* (BOOL) Close output filehandle in exit? (Default: TRUE) */
#define NP_CloseOutput	(NP_Dummy + 7)
  /* (BPTR/struct FileHandle *) Error filehandle. (Default: NIL:) */
#define NP_Error	(NP_Dummy + 8)
  /* (BOOL) Close error filehandle on exit? (Default: TRUE) */
#define NP_CloseError	(NP_Dummy + 9)

  /* (BPTR/struct FileLock *) Current directory for new task. */
#define NP_CurrentDir	(NP_Dummy + 10)
  /* (ULONG) Stacksize to use for the new process. Default is variable. */
#define NP_StackSize	(NP_Dummy + 11)
  /* (STRPTR) Name for the new process. (Default: "New Process") */
#define NP_Name 	(NP_Dummy + 12)
  /* (LONG) Priority of the new process. */
#define NP_Priority	(NP_Dummy + 13)

  /* (APTR) Pointer to the console task. */
#define NP_ConsoleTask	(NP_Dummy + 14)
  /* (struct Window *) The processes default window. */
#define NP_WindowPtr	(NP_Dummy + 15)
  /* (BPTR/struct FileLock *) The home directory of the new process. This
     defaults to the parents current directory. */
#define NP_HomeDir	(NP_Dummy + 16)
  /* (BOOL) Copy local environment variables? (Default: TRUE) */
#define NP_CopyVars	(NP_Dummy + 17)
  /* (BOOL) Create a CLI structure? (Default: FALSE) */
#define NP_Cli		(NP_Dummy + 18)

/* The following two tags are only valid for CLI processes. */
  /* (APTR) Path for the new process. */
#define NP_Path 	(NP_Dummy + 19)
  /* (STRPTR) Name of the called program. */
#define NP_CommandName	(NP_Dummy + 20)
  /* If this tag is used, NP_Input must not be NULL. */
#define NP_Arguments	(NP_Dummy + 21)

/* The following two tags do not work, yet. */
  /* (BOOL) Notify parent, when process exits? (Default: FALSE) */
#define NP_NotifyOnDeath (NP_Dummy + 22)
  /* (BOOL) Wait until called process returns. (Default: FALSE) */
#define NP_Synchronous	(NP_Dummy + 23)

  /* (APTR) Code that is to be called, when process exits. (Default: NULL) */
#define NP_ExitCode	(NP_Dummy + 24)
  /* (APTR) Optional data for NP_ExitCode. (Default: NULL) */
#define NP_ExitData	(NP_Dummy + 25)

/* The following tags are AROS specific. */
  /* (IPTR) User dependant data. Do with it, what you want to. (Default: NULL)
  */
#define NP_UserData	(NP_Dummy + 26)


/* Tags for SystemTagList(). Additionally you may use all the tags for
   CreateNewProc(). */
#if (AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
#   define SYS_Dummy       (TAG_USER + 32)
#else
#   define SYS_Dummy	   (DOS_TAGBASE)
#endif
/* The supplied filehandles are automatically closed, when the new process
   exits, if SYS_Asynch is TRUE. If it is false, they remain opened. */
  /* (BPTR/struct FileHandle *) Input filehandle. This must be a different
     filehandle from that supplied to SYS_Output. Default is Input() of the
     current process. */
#define SYS_Input	(SYS_Dummy + 1)
  /* (BPTR/struct FileHandle *) Output filehandle. This must be a different
     filehandle from than supllied to SYS_Input. Default is Output() of the
     current process. */
#define SYS_Output	(SYS_Dummy + 2)
  /* (BOOL) If FALSE, the execution of calling process is stopped, until the
     new process has finished. */
#define SYS_Asynch	(SYS_Dummy + 3)
/* If neither of the following two tags is specified, the boot-shell is used.
   SYS_UserShell takes a pointer to a shell, while CustomShell expects a
   string, which names a new shell. */
#define SYS_UserShell	(SYS_Dummy + 4) /* (BPTR) */
#define SYS_CustomShell (SYS_Dummy + 5) /* (STRPTR) */

/**********************************************************************
 **************************** Miscellaneous ***************************
 **********************************************************************/

/* Tags for AllocDosObject(). */
#if (AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
#   define ADO_Dummy	   (TAG_USER + 2000)
#else
#   define ADO_Dummy	   (DOS_TAGBASE + 2000)
#endif

/* Tags for DOS_FILEHANDLE only. */
  /* Sets up the filehandle for the specified mode. Definitions are in
     <dos/dos.h> */
#define ADO_FH_Mode	(ADO_Dummy + 1) /* Sets up FH to the specified mode. */

/* Tags for DOS_CLI. If you do not specify these, dos will use reasonable
   default values. All these tags specify the buffer length for certain
   strings. */
  /* Length of current directory buffer. */
#define ADO_DirLen	(ADO_Dummy + 2)
  /* Length of command name buffer. */
#define ADO_CommNameLen (ADO_Dummy + 3)
  /* Length of command file buffer. */
#define ADO_CommFileLen (ADO_Dummy + 4)
  /* Length of buffer for CLI prompt. */
#define ADO_PromptLen	(ADO_Dummy + 5)

#endif /* DOS_DOSTAGS_H */
