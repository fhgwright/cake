#ifndef _EXECUTE_H
#define _EXECUTE_H

/* Command symbols */
struct CommandList
{
  char * cmdsymbol;
  int  cmdnumber;
};

#define _ABORT		1
#define _ALL		2
#define _APPEND		3
#define _ASKBOOL	4
#define _ASKCHOICE	5
#define _ASKDIR		6
#define _ASKDISK	7
#define _ASKFILE	8
#define _ASKNUMBER	9
#define _ASKOPTIONS	10
#define _ASKSTRING	11
#define _ASSIGNS	12
#define _CAT		13
#define _CHOICES	14
#define _COMMAND	15
#define _COMPLETE	16
#define _CONFIRM	17
#define _COPYFILES	18
#define _COPYLIB	19
#define _DATABASE	20
#define _DEBUG		21
#define _DEFAULT	22
#define _DELETE		23
#define _DELOPTS	24
#define _DEST		25
#define _DISK		26
#define _EARLIER	27
#define _EXECUTE	28
#define _EXISTS		29
#define _EXIT		30
#define _EXPANDPATH	31
#define _FILEONLY	32
#define _FILES		33
#define _FONTS		34
#define _FOREACH	35
#define _GETASSIGN	36
#define _GETDEVICE	37
#define _GETDISKSPACE	38
#define _GETENV		39
#define _GETSIZE	40
#define _GETSUM		41
#define _GETVERSION	42
#define _HELP		43
#define _IF		44
#define _INCLUDE	45
#define _INFOS		46
#define _MAKEASSIGN	47
#define _MAKEDIR	48
#define _MESSAGE	49
#define _NEWNAME	50
#define _NEWPATH	51
#define _NOGAUGE	52
#define _NOPOSITION	53
#define _ONERROR	54
#define _OPTIONAL	55
#define _PATHONLY	56
#define _PATMATCH	57
#define _PATTERN	58
#define _PROCEDURE	59
#define _PROMPT		60
#define _PROTECT	61
#define _RANGE		62
#define _RENAME		63
#define _REXX		64
#define _RUN		65
#define _SAFE		66
#define _SELECT		67
#define _SET		68
#define _SETDEFAULTTOOL	69
#define _SETSTACK	70
#define _SETTOOLTYPE	71
#define _SHIFTLEFT	72
#define _SHIFTRGHT	73
#define _SOURCE		74
#define _STARTUP	75
#define _STRLEN		76
#define _SUBSTR		77
#define _SWAPCOLORS	78
#define _TACKON		79
#define _TEXTFILE	80
#define _TOOLTYPE	81
#define _TRANSCRIPT	82
#define _TRAP		83
#define _UNTIL		84
#define _USER		85
#define _WELCOME	86
#define _WHILE		87
#define _WORKING	88
#define _TIMES		89
#define _PLUS		90
#define _MINUS		91
#define _DIV		92
#define _LESS		93
#define _LESSEQ		94
#define _DIFF		95
#define _EQUAL		96
#define _MORE		97
#define _MOREEQ		98
#define _AND		99
#define _BITAND		100
#define _BITNOT		101
#define _BITOR		102
#define _BITXOR		103
#define _IN		104
#define _NOT		105
#define _OR		106
#define _XOR		107
#define _STRING		108
#define _QUIET		109

#define _MAXCOMMAND	109

struct CommandList internal_commands[] =
{
  {"abort",		_ABORT		},
  {"all",		_ALL		},
  {"append",		_APPEND		},
  {"askbool",		_ASKBOOL	},
  {"askchoice",		_ASKCHOICE	},
  {"askdir",		_ASKDIR		},
  {"askdisk",		_ASKDISK	},
  {"askfile",		_ASKFILE	},
  {"asknumber",		_ASKNUMBER	},
  {"askoptions",	_ASKOPTIONS	},
  {"askstring",		_ASKSTRING	},
  {"assigns",		_ASSIGNS	},
  {"cat",		_CAT		},
  {"choices",		_CHOICES	},
  {"command",		_COMMAND	},
  {"complete",		_COMPLETE	},
  {"confirm",		_CONFIRM	},
  {"copyfiles",		_COPYFILES	},
  {"copylib",		_COPYLIB	},
  {"database",		_DATABASE	},
  {"debug",		_DEBUG		},
  {"default",		_DEFAULT	},
  {"delete",		_DELETE		},
  {"delopts",		_DELOPTS	},
  {"dest",		_DEST		},
  {"disk",		_DISK		},
  {"earlier",		_EARLIER	},
  {"execute",		_EXECUTE	},
  {"exists",		_EXISTS		},
  {"exit",		_EXIT		},
  {"expandpath",	_EXPANDPATH	},
  {"fileonly",		_FILEONLY	},
  {"files",		_FILES		},
  {"fonts",		_FONTS		},
  {"foreach",		_FOREACH	},
  {"getassign",		_GETASSIGN	},
  {"getdevice",		_GETDEVICE	},
  {"getdiskspace",	_GETDISKSPACE	},
  {"getenv",		_GETENV		},
  {"getsize",		_GETSIZE	},
  {"getsum",		_GETSUM		},
  {"getversion",	_GETVERSION	},
  {"help",		_HELP		},
  {"if",		_IF		},
  {"include",		_INCLUDE	},
  {"infos",		_INFOS		},
  {"makeassign",	_MAKEASSIGN	},
  {"makedir",		_MAKEDIR	},
  {"message",		_MESSAGE	},
  {"newname",		_NEWNAME	},
  {"newpath",		_NEWPATH	},
  {"nogauge",		_NOGAUGE	},
  {"noposition",	_NOPOSITION	},
  {"onerror",		_ONERROR	},
  {"optional",		_OPTIONAL	},
  {"pathonly",		_PATHONLY	},
  {"patmatch",		_PATMATCH	},
  {"pattern",		_PATTERN	},
  {"procedure",		_PROCEDURE	},
  {"prompt",		_PROMPT		},
  {"protect",		_PROTECT	},
  {"quiet",		_QUIET		},
  {"range",		_RANGE		},
  {"rename",		_RENAME		},
  {"rexx",		_REXX		},
  {"run",		_RUN		},
  {"safe",		_SAFE		},
  {"select",		_SELECT		},
  {"set",		_SET		},
  {"setdefaulttool",	_SETDEFAULTTOOL	},
  {"setstack",		_SETSTACK	},
  {"settooltype",	_SETTOOLTYPE	},
  {"shiftleft",		_SHIFTLEFT	},
  {"shiftrght",		_SHIFTRGHT	},
  {"source",		_SOURCE		},
  {"startup",		_STARTUP	},
  {"strlen",		_STRLEN		},
  {"substr",		_SUBSTR		},
  {"swapcolors",	_SWAPCOLORS	},
  {"tackon",		_TACKON		},
  {"textfile",		_TEXTFILE	},
  {"tooltype",		_TOOLTYPE	},
  {"transcript",	_TRANSCRIPT	},
  {"trap",		_TRAP		},
  {"until",		_UNTIL		},
  {"user",		_USER		},
  {"welcome",		_WELCOME	},
  {"while",		_WHILE		},
  {"working",		_WORKING	},

  {"*",			_TIMES		},
  {"+",			_PLUS		},
  {"-",			_MINUS		},
  {"/",			_DIV		},
  {"<",			_LESS		},
  {"<=",		_LESSEQ		},
  {"<>",		_DIFF		},
  {"=",			_EQUAL		},
  {">",			_MORE		},
  {">=",		_MOREEQ		},
  {"and",		_AND		},
  {"bitand",		_BITAND		},
  {"bitnot",		_BITNOT		},
  {"bitor",		_BITOR		},
  {"bitxor",		_BITXOR		},
  {"in",		_IN		},
  {"not",		_NOT		},
  {"or",		_OR		},
  {"xor",		_XOR		}
};


#endif /* _EXECUTE_H */

