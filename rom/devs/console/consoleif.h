#ifndef CONSOLEIF_H
#define CONSOLEIF_H
/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc: Include for the console class
    Lang: english
*/

#ifndef UTILITY_TAGITEM_H
#   include <utility/tagitem.h>
#endif


enum
{
   A_Console_Window = TAG_USER + 1
};

enum 
{
    M_Console_Write = TAG_USER + 1,
    M_Console_Scroll,
    M_Console_DoCommand,
    M_Console_Left,
    M_Console_Right,
    M_Console_Up,
    M_Console_Down,
    
};

struct P_Console_Write
{
    ULONG	MethodID;
    BYTE	Command;
    UBYTE	*Params;
};

struct P_Console_ScrollDown
{
     ULONG MethodID;
     WORD  LinePos;	/* Lines including this one will be scrolled */
     
};

struct P_Console_ScrollUp
{
     ULONG MethodID;
     WORD  LinePos;	/* Lines including this one will be scrolled */
     
};

struct P_Console_DoCommand
{
    ULONG MethodID;
    BYTE Command;	/* Erase in display, scroll, next line etc.. */
    UBYTE *Params; /* The command's parameters */
};

struct P_Console_Left
{
    ULONG MethodID;
    UWORD Num;
};
struct P_Console_Right
{
    ULONG MethodID;
    UWORD Num;
};
struct P_Console_Up
{
    ULONG MethodID;
    UWORD Num;
};

struct P_Console_Down
{
    ULONG MethodID;
    UWORD Num;
};


#define Console_DoCommand(o, cmd, params)	\
({						\
	struct P_Console_Write p;		\
	p.MethodID = M_Console_DoCommand;	\
	p.Command = cmd;			\
	p.Params = params;			\
	DoMethodA((o), (Msg)&p);		\
})

#define Console_Left(o, num)		\
({					\
    struct P_Console_Left p;		\
    p.MethodID	= M_Console_Left;	\
    p.Num	= num;			\
    DoMethodA((o), (Msg)&p);		\
})


#define Console_Right(o, num)		\
({					\
    struct P_Console_Right p;		\
    p.MethodID	= M_Console_Right;	\
    p.Num	= num;			\
    DoMethodA((o), (Msg)&p);		\
})

#define Console_Up(o, num)		\
({					\
    struct P_Console_Up p;		\
    p.MethodID	= M_Console_Up;		\
    p.Num	= num;			\
    DoMethodA((o), (Msg)&p);		\
})

#define Console_Down(o, num)		\
({					\
    struct P_Console_Down p;		\
    p.MethodID	= M_Console_Down;	\
    p.Num	= num;			\
    DoMethodA((o), (Msg)&p);		\
})




#endif /* CONSOLEIF_H */
