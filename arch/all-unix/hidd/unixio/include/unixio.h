#ifndef HIDD_UNIXIO_H
#define HIDD_UNIXIO_H

/*
    Copyright (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: Unix filedescriptor/socket IO Include File
    Lang: english
*/

#ifndef HIDD_HIDD_H
#   include <hidd/hidd.h>
#endif
#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif

#define CLID_Hidd_UnixIO "unixio.hidd"
#define IID_Hidd_UnixIO	"I_Hidd_UnixIO"


enum {
    moHidd_UnixIO_Wait = 0,	/* LONG M ( uioMsg *)		*/
    moHidd_UnixIO_AsyncIO,	/* 	*/
    
    num_Hidd_UnixIO_Attrs
    
};

struct uioMsg
{
    STACKULONG um_MethodID;
    STACKULONG um_Filedesc;
    STACKULONG um_Mode;
    APTR um_CallBack;
    APTR um_CallBackData;
};

struct uioMsgAsyncIO
{
    STACKULONG um_MethodID;
    STACKULONG um_Filedesc;
    STACKULONG um_Mode;
    struct MsgPort * um_ReplyPort;
};

/* UnixIO HIDD Values */
#define vHidd_UnixIO_Read       0x1
#define vHidd_UnixIO_Write      0x2

/* Stubs */
IPTR Hidd_UnixIO_Wait(HIDD *h, ULONG fd, ULONG mode, APTR callback,  APTR callbackdata);
HIDD *New_UnixIO(struct Library * /* OOPBase */);
IPTR Hidd_UnixIO_AsyncIO(HIDD *h, ULONG fd, struct MsgPort *port, ULONG mode);

#endif /* HIDD_UNIXIO_H */


