/*
    Copyrigth 1995-2001 AROS - The Amiga Research OS
    $Id$

    Desc: reposition read/write file offset
    Lang: english
*/
#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "__errno.h"
#include "__open.h"

/*****************************************************************************

    NAME */
#include <unistd.h>

	int lseek (

/*  SYNOPSIS */
	int    filedes,
	off_t  offset,
	int    whence)

/*  FUNCTION
	Reposition read/write file offset

    INPUTS
	filedef - the filedescriptor being modified
	offset, whence -
	          How to modify the current position. whence
	  	  can be SEEK_SET, then offset is the absolute position
		  in the file (0 is the first byte), SEEK_CUR then the
		  position will change by offset (ie. -5 means to move
		  5 bytes to the beginning of the file) or SEEK_END.
		  SEEK_END means that the offset is relative to the
		  end of the file (-1 is the last byte and 0 is
		  the EOF).

    RESULT
	The new position on success and -1 on error. If an error occurred, the global
	variable errno is set.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
	fopen(), fwrite()

    INTERNALS

    HISTORY
	15.12.1996 digulla created

******************************************************************************/
{
    GETUSER;

    int  cnt;
    fdesc *fdesc = __getfdesc(filedes);

    if (!fdesc)
    {
	errno = EBADF;
	return -1;
    }

    switch (whence)
    {
    	case SEEK_SET: whence = OFFSET_BEGINNING; break;
    	case SEEK_CUR: whence = OFFSET_CURRENT; break;
    	case SEEK_END: whence = OFFSET_END; break;

	default:
	    errno = EINVAL;
	    return -1;
    }

    cnt = Seek ((BPTR)fdesc->fh, offset, whence);

    if (cnt == -1)
    	errno = IoErr2errno (IoErr ());

    return cnt + offset;
} /* lseek */
